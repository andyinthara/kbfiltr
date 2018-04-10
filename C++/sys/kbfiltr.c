/*--

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name: 

    kbfiltr.c

Abstract: This is an upper device filter driver sample for PS/2 keyboard. This
        driver layers in between the KbdClass driver and i8042prt driver and
        hooks the callback routine that moves keyboard inputs from the port
        driver to class driver. With this filter, you can remove or insert
        additional keys into the stream. This sample also creates a raw
        PDO and registers an interface so that application can talk to
        the filter driver directly without going thru the PS/2 devicestack.
        The reason for providing this additional interface is because the keyboard
        device is an exclusive secure device and it's not possible to open the
        device from usermode and send custom ioctls.

        If you want to filter keyboard inputs from all the keyboards (ps2, usb)
        plugged into the system then you can install this driver as a class filter
        and make it sit below the kbdclass filter driver by adding the service
        name of this filter driver before the kbdclass filter in the registry at
        " HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\
        {4D36E96B-E325-11CE-BFC1-08002BE10318}\UpperFilters"


Environment:

    Kernel mode only.

--*/

#include "kbfiltr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry) 
#pragma alloc_text (PAGE, KbFilter_EvtDeviceAdd)
#pragma alloc_text (PAGE, KbFilter_EvtIoInternalDeviceControl)
#endif

ULONG InstanceNo = 0;
UINT8 KeyEnabled = KEY_MODE_BINDING_ON; 

NTSTATUS
DriverEntry(
IN PDRIVER_OBJECT  DriverObject,
IN PUNICODE_STRING RegistryPath
)
/*++

Routine Description:

Installable driver initialization entry point.
This entry point is called directly by the I/O system.

Arguments:

DriverObject - pointer to the driver object

RegistryPath - pointer to a unicode string representing the path,
to driver-specific key in the registry.

Return Value:

STATUS_SUCCESS if successful,
STATUS_UNSUCCESSFUL otherwise.

--*/
{
	WDF_DRIVER_CONFIG               config;
	NTSTATUS                        status;

	DebugPrint(("Keyboard Filter Driver Sample - Driver Framework Edition.\n"));
	DebugPrint(("Built %s %s\n", __DATE__, __TIME__));

	//
	// Initiialize driver config to control the attributes that
	// are global to the driver. Note that framework by default
	// provides a driver unload routine. If you create any resources
	// in the DriverEntry and want to be cleaned in driver unload,
	// you can override that by manually setting the EvtDriverUnload in the
	// config structure. In general xxx_CONFIG_INIT macros are provided to
	// initialize most commonly used members.
	//

	WDF_DRIVER_CONFIG_INIT( &config, KbFilter_EvtDeviceAdd);

	//
	// Create a framework driver object to represent our driver.
	//
	status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE); // hDriver optional
	if (!NT_SUCCESS(status)) {
		DebugPrint(("WdfDriverCreate failed with status 0x%x\n", status));
	}

	Initialize();
	ThreadLoadConfig();

	return status;
}

NTSTATUS
KbFilter_EvtDeviceAdd(
IN WDFDRIVER        Driver,
IN PWDFDEVICE_INIT  DeviceInit
)
/*++
Routine Description:

EvtDeviceAdd is called by the framework in response to AddDevice
call from the PnP manager. Here you can query the device properties
using WdfFdoInitWdmGetPhysicalDevice/IoGetDeviceProperty and based
on that, decide to create a filter device object and attach to the
function stack.

If you are not interested in filtering this particular instance of the
device, you can just return STATUS_SUCCESS without creating a framework
device.

Arguments:

Driver - Handle to a framework driver object created in DriverEntry

DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

NTSTATUS

--*/
{
	WDF_OBJECT_ATTRIBUTES   deviceAttributes;
	NTSTATUS                status;
	WDFDEVICE               hDevice;
	WDFQUEUE                hQueue;
	PDEVICE_EXTENSION       filterExt;
	WDF_IO_QUEUE_CONFIG     ioQueueConfig;

	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();

	DebugPrint(("Enter FilterEvtDeviceAdd \n"));

	//
	// Tell the framework that you are filter driver. Framework
	// takes care of inherting all the device flags & characterstics
	// from the lower device you are attaching to.
	//
	WdfFdoInitSetFilter(DeviceInit);

	WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_KEYBOARD);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_EXTENSION);

	//
	// Create a framework device object.  This call will in turn create
	// a WDM deviceobject, attach to the lower stack and set the
	// appropriate flags and attributes.
	//
	status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &hDevice);
	if (!NT_SUCCESS(status)) {
		DebugPrint(("WdfDeviceCreate failed with status code 0x%x\n", status));
		return status;
	}

	filterExt = FilterGetData(hDevice);

	//
	// Configure the default queue to be Parallel. Do not use sequential queue
	// if this driver is going to be filtering PS2 ports because it can lead to
	// deadlock. The PS2 port driver sends a request to the top of the stack when it
	// receives an ioctl request and waits for it to be completed. If you use a
	// a sequential queue, this request will be stuck in the queue because of the 
	// outstanding ioctl request sent earlier to the port driver.
	//
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchParallel);

	//
	// Framework by default creates non-power managed queues for
	// filter drivers.
	//
	ioQueueConfig.EvtIoInternalDeviceControl = KbFilter_EvtIoInternalDeviceControl;

	status = WdfIoQueueCreate(hDevice, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE); // pointer to default queue
	if (!NT_SUCCESS(status)) {
		DebugPrint(("WdfIoQueueCreate failed 0x%x\n", status));
		return status;
	}

	//
	// Create a new queue to handle IOCTLs that will be forwarded to us from
	// the rawPDO. 
	//
	WDF_IO_QUEUE_CONFIG_INIT(&ioQueueConfig, WdfIoQueueDispatchParallel);

	//
	// Framework by default creates non-power managed queues for
	// filter drivers.
	//
	ioQueueConfig.EvtIoDeviceControl = KbFilter_EvtIoDeviceControlFromRawPdo;

	status = WdfIoQueueCreate(hDevice, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &hQueue);
	if (!NT_SUCCESS(status)) {
		DebugPrint(("WdfIoQueueCreate failed 0x%x\n", status));
		return status;
	}

	filterExt->rawPdoQueue = hQueue;

	//
	// Create a RAW pdo so we can provide a sideband communication with
	// the application. Please note that not filter drivers desire to
	// produce such a communication and not all of them are contrained
	// by other filter above which prevent communication thru the device
	// interface exposed by the main stack. So use this only if absolutely
	// needed. Also look at the toaster filter driver sample for an alternate
	// approach to providing sideband communication.
	//
	status = KbFiltr_CreateRawPdo(hDevice, ++InstanceNo);

	return status;
}

VOID
KbFilter_EvtIoDeviceControlFromRawPdo(
IN WDFQUEUE      Queue,
IN WDFREQUEST    Request,
IN size_t        OutputBufferLength,
IN size_t        InputBufferLength,
IN ULONG         IoControlCode
)
/*++

Routine Description:

This routine is the dispatch routine for device control requests.

Arguments:

Queue - Handle to the framework queue object that is associated
with the I/O request.
Request - Handle to a framework request object.

OutputBufferLength - length of the request's output buffer,
if an output buffer is available.
InputBufferLength - length of the request's input buffer,
if an input buffer is available.

IoControlCode - the driver-defined or system-defined I/O control code
(IOCTL) that is associated with the request.

Return Value:

VOID

--*/
{
	NTSTATUS status = STATUS_SUCCESS;
	WDFDEVICE hDevice;
	WDFMEMORY outputMemory;
	PDEVICE_EXTENSION devExt;
	size_t bytesTransferred = 0;

	UNREFERENCED_PARAMETER(InputBufferLength);

	DebugPrint(("Entered KbFilter_EvtIoInternalDeviceControl\n"));

	hDevice = WdfIoQueueGetDevice(Queue);
	devExt = FilterGetData(hDevice);

	//
	// Process the ioctl and complete it when you are done.
	//

	switch (IoControlCode) {
	case IOCTL_KBFILTR_GET_KEYBOARD_ATTRIBUTES:

		//
		// Buffer is too small, fail the request
		//
		if (OutputBufferLength < sizeof(KEYBOARD_ATTRIBUTES)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);

		if (!NT_SUCCESS(status)) {
			DebugPrint(("WdfRequestRetrieveOutputMemory failed %x\n", status));
			break;
		}

		status = WdfMemoryCopyFromBuffer(outputMemory,
			0,
			&devExt->KeyboardAttributes,
			sizeof(KEYBOARD_ATTRIBUTES));

		if (!NT_SUCCESS(status)) {
			DebugPrint(("WdfMemoryCopyFromBuffer failed %x\n", status));
			break;
		}

		bytesTransferred = sizeof(KEYBOARD_ATTRIBUTES);

		break;
	default:
		status = STATUS_NOT_IMPLEMENTED;
		break;
	}

	WdfRequestCompleteWithInformation(Request, status, bytesTransferred);

	return;
}

VOID
KbFilter_EvtIoInternalDeviceControl(
IN WDFQUEUE      Queue,
IN WDFREQUEST    Request,
IN size_t        OutputBufferLength,
IN size_t        InputBufferLength,
IN ULONG         IoControlCode
)
/*++

Routine Description:

This routine is the dispatch routine for internal device control requests.
There are two specific control codes that are of interest:

IOCTL_INTERNAL_KEYBOARD_CONNECT:
Store the old context and function pointer and replace it with our own.
This makes life much simpler than intercepting IRPs sent by the RIT and
modifying them on the way back up.

IOCTL_INTERNAL_I8042_HOOK_KEYBOARD:
Add in the necessary function pointers and context values so that we can
alter how the ps/2 keyboard is initialized.

NOTE:  Handling IOCTL_INTERNAL_I8042_HOOK_KEYBOARD is *NOT* necessary if
all you want to do is filter KEYBOARD_INPUT_DATAs.  You can remove
the handling code and all related device extension fields and
functions to conserve space.

Arguments:

Queue - Handle to the framework queue object that is associated
with the I/O request.
Request - Handle to a framework request object.

OutputBufferLength - length of the request's output buffer,
if an output buffer is available.
InputBufferLength - length of the request's input buffer,
if an input buffer is available.

IoControlCode - the driver-defined or system-defined I/O control code
(IOCTL) that is associated with the request.

Return Value:

VOID

--*/
{
	PDEVICE_EXTENSION               devExt;
	PINTERNAL_I8042_HOOK_KEYBOARD   hookKeyboard = NULL;
	PCONNECT_DATA                   connectData = NULL;
	NTSTATUS                        status = STATUS_SUCCESS;
	size_t                          length;
	WDFDEVICE                       hDevice;
	BOOLEAN                         forwardWithCompletionRoutine = FALSE;
	BOOLEAN                         ret = TRUE;
	WDFCONTEXT                      completionContext = WDF_NO_CONTEXT;
	WDF_REQUEST_SEND_OPTIONS        options;
	WDFMEMORY                       outputMemory;
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);


	PAGED_CODE();

	DebugPrint(("Entered KbFilter_EvtIoInternalDeviceControl\n"));

	hDevice = WdfIoQueueGetDevice(Queue);
	devExt = FilterGetData(hDevice);

	switch (IoControlCode) {

		//
		// Connect a keyboard class device driver to the port driver.
		//
	case IOCTL_INTERNAL_KEYBOARD_CONNECT:
		//
		// Only allow one connection.
		//
		if (devExt->UpperConnectData.ClassService != NULL) {
			status = STATUS_SHARING_VIOLATION;
			break;
		}

		//
		// Get the input buffer from the request
		// (Parameters.DeviceIoControl.Type3InputBuffer).
		//
		status = WdfRequestRetrieveInputBuffer(Request, sizeof(CONNECT_DATA), &connectData, &length);
		if (!NT_SUCCESS(status)){
			DebugPrint(("WdfRequestRetrieveInputBuffer failed %x\n", status));
			break;
		}

		NT_ASSERT(length == InputBufferLength);

		devExt->UpperConnectData = *connectData;

		//
		// Hook into the report chain.  Everytime a keyboard packet is reported
		// to the system, KbFilter_ServiceCallback will be called
		//

		connectData->ClassDeviceObject = WdfDeviceWdmGetDeviceObject(hDevice);

#pragma warning(disable:4152)  //nonstandard extension, function/data pointer conversion

		connectData->ClassService = KbFilter_ServiceCallback;

#pragma warning(default:4152)

		break;

		//
		// Disconnect a keyboard class device driver from the port driver.
		//
	case IOCTL_INTERNAL_KEYBOARD_DISCONNECT:

		//
		// Clear the connection parameters in the device extension.
		//
		// devExt->UpperConnectData.ClassDeviceObject = NULL;
		// devExt->UpperConnectData.ClassService = NULL;

		status = STATUS_NOT_IMPLEMENTED;
		break;

		//
		// Attach this driver to the initialization and byte processing of the
		// i8042 (ie PS/2) keyboard.  This is only necessary if you want to do PS/2
		// specific functions, otherwise hooking the CONNECT_DATA is sufficient
		//
	case IOCTL_INTERNAL_I8042_HOOK_KEYBOARD:

		DebugPrint(("hook keyboard received!\n"));

		//
		// Get the input buffer from the request
		// (Parameters.DeviceIoControl.Type3InputBuffer)
		//
		status = WdfRequestRetrieveInputBuffer(Request, sizeof(INTERNAL_I8042_HOOK_KEYBOARD), &hookKeyboard, &length);
		if (!NT_SUCCESS(status)){
			DebugPrint(("WdfRequestRetrieveInputBuffer failed %x\n", status));
			break;
		}

		NT_ASSERT(length == InputBufferLength);

		//
		// Enter our own initialization routine and record any Init routine
		// that may be above us.  Repeat for the isr hook
		//
		devExt->UpperContext = hookKeyboard->Context;

		//
		// replace old Context with our own
		//
		hookKeyboard->Context = (PVOID)devExt;

		if (hookKeyboard->InitializationRoutine) {
			devExt->UpperInitializationRoutine =
				hookKeyboard->InitializationRoutine;
		}
		hookKeyboard->InitializationRoutine =
			(PI8042_KEYBOARD_INITIALIZATION_ROUTINE)
			KbFilter_InitializationRoutine;

		if (hookKeyboard->IsrRoutine) {
			devExt->UpperIsrHook = hookKeyboard->IsrRoutine;
		}
		hookKeyboard->IsrRoutine = (PI8042_KEYBOARD_ISR)KbFilter_IsrHook;

		//
		// Store all of the other important stuff
		//
		devExt->IsrWritePort = hookKeyboard->IsrWritePort;
		devExt->QueueKeyboardPacket = hookKeyboard->QueueKeyboardPacket;
		devExt->CallContext = hookKeyboard->CallContext;

		status = STATUS_SUCCESS;
		break;


	case IOCTL_KEYBOARD_QUERY_ATTRIBUTES:
		forwardWithCompletionRoutine = TRUE;
		completionContext = devExt;
		break;

		//
		// Might want to capture these in the future.  For now, then pass them down
		// the stack.  These queries must be successful for the RIT to communicate
		// with the keyboard.
		//
	case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION:
	case IOCTL_KEYBOARD_QUERY_INDICATORS:
	case IOCTL_KEYBOARD_SET_INDICATORS:
	case IOCTL_KEYBOARD_QUERY_TYPEMATIC:
	case IOCTL_KEYBOARD_SET_TYPEMATIC:
		break;
	}

	if (!NT_SUCCESS(status)) {
		WdfRequestComplete(Request, status);
		return;
	}

	//
	// Forward the request down. WdfDeviceGetIoTarget returns
	// the default target, which represents the device attached to us below in
	// the stack.
	//

	if (forwardWithCompletionRoutine) {
		//
		// Format the request with the output memory so the completion routine
		// can access the return data in order to cache it into the context area
		//

		status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);

		if (!NT_SUCCESS(status)) {
			DebugPrint(("WdfRequestRetrieveOutputMemory failed: 0x%x\n", status));
			WdfRequestComplete(Request, status);
			return;
		}

		status = WdfIoTargetFormatRequestForInternalIoctl(WdfDeviceGetIoTarget(hDevice), Request, IoControlCode, NULL, NULL, outputMemory, NULL);

		if (!NT_SUCCESS(status)) {
			DebugPrint(("WdfIoTargetFormatRequestForInternalIoctl failed: 0x%x\n", status));
			WdfRequestComplete(Request, status);
			return;
		}

		// 
		// Set our completion routine with a context area that we will save
		// the output data into
		//
		WdfRequestSetCompletionRoutine(Request, KbFilterRequestCompletionRoutine, completionContext);

		ret = WdfRequestSend(Request, WdfDeviceGetIoTarget(hDevice), WDF_NO_SEND_OPTIONS);

		if (ret == FALSE) {
			status = WdfRequestGetStatus(Request);
			DebugPrint(("WdfRequestSend failed: 0x%x\n", status));
			WdfRequestComplete(Request, status);
		}
	}
	else
	{
		//
		// We are not interested in post processing the IRP so 
		// fire and forget.
		//
		WDF_REQUEST_SEND_OPTIONS_INIT(&options,
			WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);

		ret = WdfRequestSend(Request, WdfDeviceGetIoTarget(hDevice), &options);

		if (ret == FALSE) {
			status = WdfRequestGetStatus(Request);
			DebugPrint(("WdfRequestSend failed: 0x%x\n", status));
			WdfRequestComplete(Request, status);
		}
	}

	return;
}


VOID
KbFilterRequestCompletionRoutine(
WDFREQUEST                  Request,
WDFIOTARGET                 Target,
PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
WDFCONTEXT                  Context
)
/*++

Routine Description:

Completion Routine

Arguments:

Target - Target handle
Request - Request handle
Params - request completion params
Context - Driver supplied context


Return Value:

VOID

--*/
{
	WDFMEMORY   buffer = CompletionParams->Parameters.Ioctl.Output.Buffer;
	NTSTATUS    status = CompletionParams->IoStatus.Status;

	UNREFERENCED_PARAMETER(Target);

	//
	// Save the keyboard attributes in our context area so that we can return
	// them to the app later.
	//
	if (NT_SUCCESS(status) &&
		CompletionParams->Type == WdfRequestTypeDeviceControlInternal &&
		CompletionParams->Parameters.Ioctl.IoControlCode == IOCTL_KEYBOARD_QUERY_ATTRIBUTES) {

		if (CompletionParams->Parameters.Ioctl.Output.Length >= sizeof(KEYBOARD_ATTRIBUTES)) {

			status = WdfMemoryCopyToBuffer(buffer,
				CompletionParams->Parameters.Ioctl.Output.Offset,
				&((PDEVICE_EXTENSION)Context)->KeyboardAttributes,
				sizeof(KEYBOARD_ATTRIBUTES)
				);
		}
	}

	WdfRequestComplete(Request, status);
	return;
}



NTSTATUS
KbFilter_InitializationRoutine(
IN PVOID                           InitializationContext,
IN PVOID                           SynchFuncContext,
IN PI8042_SYNCH_READ_PORT          ReadPort,
IN PI8042_SYNCH_WRITE_PORT         WritePort,
OUT PBOOLEAN                       TurnTranslationOn
)
/*++

Routine Description:

This routine gets called after the following has been performed on the kb
1)  a reset
2)  set the typematic
3)  set the LEDs

i8042prt specific code, if you are writing a packet only filter driver, you
can remove this function

Arguments:

DeviceObject - Context passed during IOCTL_INTERNAL_I8042_HOOK_KEYBOARD

SynchFuncContext - Context to pass when calling Read/WritePort

Read/WritePort - Functions to synchronoulsy read and write to the kb

TurnTranslationOn - If TRUE when this function returns, i8042prt will not
turn on translation on the keyboard

Return Value:

Status is returned.

--*/
{
	PDEVICE_EXTENSION   devExt;
	NTSTATUS            status = STATUS_SUCCESS;

	devExt = (PDEVICE_EXTENSION)InitializationContext;

	//
	// Do any interesting processing here.  We just call any other drivers
	// in the chain if they exist.  Make sure Translation is turned on as well
	//

	if (devExt->UpperInitializationRoutine) {
		status = (*devExt->UpperInitializationRoutine) (
			devExt->UpperContext,
			SynchFuncContext,
			ReadPort,
			WritePort,
			TurnTranslationOn
			);

		if (!NT_SUCCESS(status)) {
			return status;
		}
	}

	*TurnTranslationOn = TRUE;

	if (KeyEnabled == 0 || reload_config == SETTING_ON) ThreadLoadConfig();
	return status;
}

BOOLEAN
KbFilter_IsrHook(
PVOID                  IsrContext,
PKEYBOARD_INPUT_DATA   CurrentInput,
POUTPUT_PACKET         CurrentOutput,
UCHAR                  StatusByte,
PUCHAR                 DataByte,
PBOOLEAN               ContinueProcessing,
PKEYBOARD_SCAN_STATE   ScanState
)
/*++

Routine Description:

This routine gets called at the beginning of processing of the kb interrupt.

i8042prt specific code, if you are writing a packet only filter driver, you
can remove this function

Arguments:

DeviceObject - Our context passed during IOCTL_INTERNAL_I8042_HOOK_KEYBOARD

CurrentInput - Current input packet being formulated by processing all the
interrupts

CurrentOutput - Current list of bytes being written to the keyboard or the
i8042 port.

StatusByte    - Byte read from I/O port 60 when the interrupt occurred

DataByte      - Byte read from I/O port 64 when the interrupt occurred.
This value can be modified and i8042prt will use this value
if ContinueProcessing is TRUE

ContinueProcessing - If TRUE, i8042prt will proceed with normal processing of
the interrupt.  If FALSE, i8042prt will return from the
interrupt after this function returns.  Also, if FALSE,
it is this functions responsibility to report the input
packet via the function provided in the hook IOCTL or via
queueing a DPC within this driver and calling the
service callback function acquired from the connect IOCTL

Return Value:

Status is returned.

--*/
{
	PDEVICE_EXTENSION devExt;
	BOOLEAN           retVal = TRUE;


	if (KeyEnabled == 0 || reload_config == SETTING_ON) ThreadLoadConfig();

	devExt = (PDEVICE_EXTENSION)IsrContext;

	if (devExt->UpperIsrHook) {
		retVal = (*devExt->UpperIsrHook) ( devExt->UpperContext, CurrentInput, CurrentOutput, StatusByte, DataByte, ContinueProcessing, ScanState );

		if (!retVal || !(*ContinueProcessing)) {
			return retVal;
		}
	}
	
	*ContinueProcessing = TRUE;
	
	return retVal;
}



VOID
KbFilter_ServiceCallback(
IN PDEVICE_OBJECT  DeviceObject,
IN PKEYBOARD_INPUT_DATA InputDataStart,
IN PKEYBOARD_INPUT_DATA InputDataEnd,
IN OUT PULONG InputDataConsumed
)
/*++

Routine Description:

Called when there are keyboard packets to report to the Win32 subsystem.
You can do anything you like to the packets.  For instance:

o Drop a packet altogether
o Mutate the contents of a packet
o Insert packets into the stream

Arguments:

DeviceObject - Context passed during the connect IOCTL

InputDataStart - First packet to be reported

InputDataEnd - One past the last packet to be reported.  Total number of
packets is equal to InputDataEnd - InputDataStart

InputDataConsumed - Set to the total number of packets consumed by the RIT
(via the function pointer we replaced in the connect
IOCTL)

Return Value:

Status is returned.

--*/
{
	WDFDEVICE hDevice = WdfWdmDeviceGetWdfDeviceHandle(DeviceObject);
	PDEVICE_EXTENSION devExt = FilterGetData(hDevice);
	size_t length = InputDataEnd - InputDataStart;
	KEYBOARD_INPUT_DATA data[MAX_KB_INPUT_DATA];
	USHORT keyrepeat = 0, processbinding, longkeybinding = 0;
	LARGE_INTEGER tickcount;

	kcount = 0;

	for (size_t i = 0; i < length; i++)
	{
		processbinding = FALSE;


		// SCROLLOCK to cycle through modes
		if (InputDataStart[i].MakeCode == K_SCROLLLOCK)
		{
			if (InputDataStart[i].Flags == KEY_BREAK)
			{
				if (++KeyEnabled > 3) KeyEnabled = KEY_MODE_KEYBOARD_OFF;
			}
			continue;
		}



		if (KeyEnabled == KEY_MODE_BINDING_ON)
		{
			last_packet_length = length;
			USHORT keyp = InputDataStart[i].MakeCode;
			if (keyp == 0) continue;

			USHORT keystate = InputDataStart[i].Flags;
			kcount = 0;

			if (layer == 0)
			{
				if (bindings[layer][keyp][K_SINGLE].out1) // single key pre-filters 
				{
					InputDataStart[i].MakeCode = keyp = bindings[layer][keyp][K_SINGLE].out1;
					InputDataStart[i].Flags = InputDataStart[i].Flags | ((keyp >= K_HOME) ? KEY_E0 : 0);
				}
			}


			if ((keystate & KEY_BREAK) == KEY_BREAK)
			{
				if (keyp == key1) // at this point keyp is non-zero, key1 pressed and being released
				{
					if (key2 == 0) // at this point key1 is non-zero, no binding started
					{
						if (bindings[layer][key1][key1].out1) // long key binding
						{
							KeQueryTickCount(&tickcount);
							if ((tickcount.QuadPart - khold) >= longkey_time)
							{
								key1 = key2 = keyp;
								processbinding = TRUE;
								longkeybinding = TRUE;
							}
						}

						if (!processbinding)
						{
							keyout[kcount++] = Keydata(key1, KEY_MAKE); keyout[kcount++] = Keydata(key1, KEY_BREAK);
						}
					}

					if (!processbinding) key1 = 0;
					khold = 0;
				}
				keyrepeat = 0;
			}
			else if (((keystate & KEY_MAKE) == KEY_MAKE) && keyrepeat == 0)
			{
				if (key1 == 0)
				{
					// no key binding when starting with shift key
					if ((keyp != K_LSHIFT) && (keyp != K_RSHIFT))
					{
						key2 = 0;

						if (bindings[layer][keyp][K_ENABLED].out1) // binding found, start the wait
						{
							key1 = keyp;
							if (bindings[layer][key1][K_SINGLE].out1) // check if it's a key combo or a single-single key binding
							{
								key2 = K_SINGLE;
								processbinding = TRUE;
							}
							else // hold key1 and wait for key2 press
							{
								KeQueryTickCount(&tickcount);
								khold = tickcount.QuadPart;
								continue;
							}
						}
					}
				}
				else // key1 != 0
				{
					KeQueryTickCount(&tickcount);
					if (key1 == keyp) // key holding
					{
						if (!bindings[layer][key1][key1].out1) // long key binding
						{
							if ((tickcount.QuadPart - khold) >= key_repeat_time)
							{
								keyrepeat = 1; // if key has been held for longer than repeat time then do nothing and let the key through for native repeating
							}
							else continue;
						}
						continue;
					}
					else
					{
						// key_bind_time has ellapsed, binding matched for key1 + key2
						if (bindings[layer][key1][keyp].out1) // have out1
						{
							if ((tickcount.QuadPart - khold) >= key_bind_time)
							{
								key2 = keyp;
								processbinding = TRUE;
							}
							else // key_bind_time has not ellapsed, no binding registered, output the held keys as normal keypresses
							{
								keyout[kcount++] = Keydata(key1, KEY_MAKE); keyout[kcount++] = Keydata(key1, KEY_BREAK);
								keyout[kcount++] = Keydata(keyp, KEY_MAKE); keyout[kcount++] = Keydata(keyp, KEY_BREAK);
								key1 = 0;
								key2 = 0;
							}
						}
						else // no binding found, output keyp
						{
							if (key2) // binding has already started
							{
								keyout[kcount++] = Keydata(keyp, KEY_MAKE); keyout[kcount++] = Keydata(keyp, KEY_BREAK);
								key2 = keyp;
							}
							else // if no binding has started just treat it as quick sequential keystrokes
							{
								keyout[kcount++] = Keydata(key1, KEY_MAKE); keyout[kcount++] = Keydata(key1, KEY_BREAK);
								keyout[kcount++] = Keydata(keyp, KEY_MAKE); keyout[kcount++] = Keydata(keyp, KEY_BREAK);
								key1 = 0;
								key2 = 0;
							}
						}
					}
				}
			}






			if (processbinding)
			{
				if (bindings[layer][key1][key2].out1 == K_VARIABLE) // binding starting with ~ is an internal command
				{
					Command(bindings[layer][key1][key2].out2, key1, key2, bindings[layer][key1][key2]);
					key1 = 0; key2 = 0;
					continue; // no output
				}
				else if (bindings[layer][key1][key2].out1 == K_COMMAND) // stored command(s) 
				{
					char *command = commands[bindings[layer][key1][key2].out2]; // out2 => command sequence name 
					for (int ci = 0; ci < COMMAND_LEN; ci++)
					{
						char command_name = command[ci];
						if (command_name == 0) continue;
						Keyoutput(layer, K_COMMAND, keymap[command_name]);
					}
				}
				else
				{
					Keyoutput(layer, key1, key2);
				}
				processbinding = FALSE;
				
				if (key2 == K_SINGLE) keyrepeat = 0;
				if (longkeybinding) longkeybinding = key1 = key2 = 0;
			}

			
			if (pause == TRUE) continue; // pause command


			if (kcount)
			{
				for (USHORT j = 0; j < kcount; j++)
				{
					USHORT key = keyout[j].key;
					data[j].MakeCode = key;
					data[j].Flags = keyout[j].flag | ((key >= K_HOME) ? KEY_E0 : 0);
				}
				ULONG _consumed = kcount;
				PULONG consumed = &_consumed;

				(*(PSERVICE_CALLBACK_ROUTINE)(ULONG_PTR)devExt->UpperConnectData.ClassService)(
					devExt->UpperConnectData.ClassDeviceObject, &data[0], &data[kcount], consumed);
			}
			else
			{
				(*(PSERVICE_CALLBACK_ROUTINE)(ULONG_PTR)devExt->UpperConnectData.ClassService)(
					devExt->UpperConnectData.ClassDeviceObject, &InputDataStart[i], &InputDataStart[i+1], InputDataConsumed);
			}
		}
		else if (KeyEnabled == KEY_MODE_BINDING_OFF)
		{
			(*(PSERVICE_CALLBACK_ROUTINE)(ULONG_PTR)devExt->UpperConnectData.ClassService)(
				devExt->UpperConnectData.ClassDeviceObject, &InputDataStart[i], &InputDataStart[i + 1], InputDataConsumed);
		}
		else
		{
			// diagnostic mode
			// no keys will register
			// but certain command keys will output setting values
			USHORT keyp = InputDataStart[i].MakeCode;
			USHORT keystate = InputDataStart[i].Flags;
			ULONG _consumed = 0;
			PULONG consumed = &_consumed;
			kcount = 0;
			char message[MSG_LEN];
			memset(message, 0, MSG_LEN);

			if (keystate == KEY_MAKE && keyp == K_H)
			{
				sprintf(message, "t=%luSdt=%luSr=%luSo=%luSs=%luSc=%luS",
					key_bind_time, longkey_time, key_repeat_time, key_bind_timeout, safe_mode, capslock_to_lshift);
			}
			else if (keystate == KEY_MAKE && keyp == K_J)
			{
				sprintf(message, "l=%luSr=%luSlc=%luSpl=%luS1=%luk=%lu",
					layer, reload_config, loading_config, last_packet_length, key1, krelease);
			}

			int c = 0;
			char k = message[c++];
			while (k && (c < MSG_LEN))
			{
				USHORT key = keymap[k];
				keyout[kcount++] = Keydata(key, KEY_MAKE);
				keyout[kcount++] = Keydata(key, KEY_BREAK);
				k = message[c++];
			}

			if (kcount)
			{
				for (USHORT j = 0; j < kcount; j++)
				{
					USHORT key = keyout[j].key;
					data[j].MakeCode = key;
					data[j].Flags = keyout[j].flag | ((key >= K_HOME) ? KEY_E0 : 0);
				}
				*consumed = kcount;

				(*(PSERVICE_CALLBACK_ROUTINE)(ULONG_PTR)devExt->UpperConnectData.ClassService)(
					devExt->UpperConnectData.ClassDeviceObject, &data[0], &data[kcount], consumed);
			}
		}
	}

	*InputDataConsumed = InputDataEnd - InputDataStart;
}


VOID Keyoutput(USHORT layer, USHORT key1, USHORT key2)
{
	if (bindings[layer][key1][key2].out1 == K_VARIABLE) // binding starting with ~ is an internal command
	{
		Command(bindings[layer][key1][key2].out2, key1, key2, bindings[layer][key1][key2]);
	}
	else if (bindings[layer][key1][key2].out3) // 3 key output	
	{
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out1, KEY_MAKE);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out2, KEY_MAKE);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out3, KEY_MAKE);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out3, KEY_BREAK);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out2, KEY_BREAK);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out1, KEY_BREAK);
	}
	else if (bindings[layer][key1][key2].out2) // 2 key output	
	{
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out1, KEY_MAKE);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out2, KEY_MAKE);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out2, KEY_BREAK);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out1, KEY_BREAK);
	}
	else // 1 key output
	{
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out1, KEY_MAKE | bindings[layer][key1][key2].flag1);
		keyout[kcount++] = Keydata(bindings[layer][key1][key2].out1, KEY_BREAK | bindings[layer][key1][key2].flag1);
	}
}


VOID Command(USHORT cmd, USHORT key1, USHORT key2, struct_binding binding)
{
	key1;
	key2;
	switch (cmd)
	{
	case K_L: // layer change
		layer = binding.arg1;
		break;

	case K_P: // pause keyboard
		if (pause == FALSE) pause = TRUE;
		else pause = FALSE;
		break;

	case K_1: // change to mode 1
		break;

	case K_R: // Reload config file 
		reload_config = SETTING_ON;
		break;

	case K_C: // command combo
		// call one by one up to 
		break;
	}
}




keydata Keydata(USHORT k, USHORT f)
{
	keydata o;
	o.key = k;
	o.flag = f;
	return o;
}




VOID ThreadLoadConfig()
{
	if (loading_config == 1) return;
	if (loading_config == 2)
	{
		reload_config = SETTING_OFF;
		KeyEnabled = KEY_MODE_BINDING_ON;
		loading_config = 0; 
		return;
	}

	HANDLE hthread;
	if (PsCreateSystemThread(&hthread, THREAD_ALL_ACCESS, NULL, NULL, NULL, (PKSTART_ROUTINE)LoadConfig, NULL))
	{
		loading_config = 0;
		KeyEnabled = KEY_MODE_BINDING_ON;
	}
}

INT LoadConfig()
{
	HANDLE   handle;
	NTSTATUS ntstatus;
	IO_STATUS_BLOCK    ioStatusBlock;
	LARGE_INTEGER      byteOffset;

	// Do not try to perform any file operations at higher IRQL levels.
	// Instead, you may use a work item or a system worker thread to perform file operations.
	UNICODE_STRING     uniName;
	OBJECT_ATTRIBUTES  objAttr;

	RtlInitUnicodeString(&uniName, L"\\SystemRoot\\kbfiltr.txt");
	InitializeObjectAttributes(&objAttr, &uniName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	
	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
	{
		loading_config = 2;
		PsTerminateSystemThread(STATUS_SUCCESS);
		return STATUS_INVALID_DEVICE_STATE;
	}


	ntstatus = ZwCreateFile(&handle,
		GENERIC_READ, &objAttr, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

	if (NT_SUCCESS(ntstatus))
	{
		byteOffset.LowPart = byteOffset.HighPart = 0;
		ntstatus = ZwReadFile(handle, NULL, NULL, NULL, &ioStatusBlock, buffer, CONFIG_BUFFER_SIZE, &byteOffset, NULL);
		if (NT_SUCCESS(ntstatus))
		{
			buffer[CONFIG_BUFFER_SIZE - 1] = '\0';

			// parse the file
			// key bindings
			// key binding for each key with no function key
			// bindings lay on top of the standard QWERTY keys
			// syntax: 
			// " signifies start of comment
			// # hold <a> then <;> to trigger <enter>
			// a; N	" hold a + ; = enter
			// af E " hold a + f = esc 
			// ~l 1	" set layer to 1, subsequent bindings will be for this layer 1
			// ~l 2	" set layer to 2, subsequent bindings will be for this layer 2
			// ~l 0	" set layer to 0, subsequent bindings will be for this layer 0
			// ~l * " set for all layers 
			// vj ~l 0 " ~ = function call, l = change layer, 0 = argument (layer 0)
			// j  2	" single key binding: key<space><space>binding
			// jj e " key hold binding: hold j for long key time for 'e' binding
			// Q " stored command for recall

			int len = strlen(buffer);
			BOOLEAN commenton = FALSE;
			int i = 0;
			int l = 0;
			int cmdlen = 0;
			int _layer = 0;
			USHORT key1 = 0, key2 = 0;
			USHORT cmd[CMD_LEN];
			memset(cmd, 0, CMD_LEN);
			memset(bindings, 0, sizeof(struct_binding) * MAX_LAYERS * MAX_KEYS * MAX_KEYS);
			memset(phrases, 0, PHRASE_MAX * PHRASE_LEN);

			while (i < len)
			{
				char c = buffer[i++];
				if (c == '\r' || c == '\t') continue;

				if (c == '\n' || c == '\0')
				{
					if (key1)
					{
						if (key1 == K_VARIABLE) // ~
						{
							// use the file to set some system variables
							char str[CMD_LEN];
							memset(str, 0, CMD_LEN);
							memcpy(str, cmd, CMD_LEN);

							// Key bind time 
							if (key2 == K_T)
							{
								ULONG value = atoi(str);
								key_bind_time = value;
							}
							else if (key2 == K_D) // longkey time
							{
								ULONG value = atoi(str);
								longkey_time = value;
							}
							else if (key2 == K_R) // key repeat time
							{
								ULONG value = atoi(str);
								key_repeat_time = value;
							}
							else if (key2 == K_O) // key timeout
							{
								ULONG value = atoi(str);
								key_bind_timeout = value;
							}
							else if (key2 == K_S) // safe mode during config load 1 = on 0 = off 
							{
								ULONG value = atoi(str);
								safe_mode = value;
							}
							else if (key2 == K_C) // 1 capslock works as left shift, 0 use capslock as normal 
							{
								ULONG value = atoi(str);
								capslock_to_lshift = value;
							}
							else if (key2 == K_L) // set binding layer 
							{
								// all layers
								if (str[0] == '*') _layer = ALL_LAYERS;
								else
								{
									ULONG value = atoi(str);
									_layer = min(value, MAX_LAYERS - 1);
								}
							}
							else if (key2 == K_COMMAND) // set command sequence  
							{
								memcpy(commands[keymap[str[0]]], &str[1], COMMAND_LEN);
							}
							else
							{
								// TODO combo hotkey phrase inserts
								// TODO control mouse buttons and movement
							}
						}
						else
						{
							if (cmdlen >= 4 && cmd[0] != '~') // key binding by HEX codes 
							{
								// 2 byte HEX codes = 4 characters
								//example E05B = LWIN
								int start_layer = 0, end_layer = MAX_LAYERS - 1;
								if (_layer != ALL_LAYERS) start_layer = end_layer = _layer;
								for (int i = start_layer; i <= end_layer; i++)
								{
									bindings[i][key1][K_ENABLED].out1 = K_BOUND;
									bindings[i][key1][key2].out1 = (hex2dec[cmd[2]] << 4) + hex2dec[cmd[3]];
									bindings[i][key1][key2].flag1 = (hex2dec[cmd[0]] << 4) + hex2dec[cmd[1]];
									bindings[i][key1][key2].out2 = 0;
								}
							}
							else // key binding
							{
								if (key2 == K_UNDEFINED) key2 = K_SINGLE;

								int start_layer = 0, end_layer = MAX_LAYERS - 1;
								if (_layer != ALL_LAYERS) start_layer = end_layer = _layer;
								for (int i = start_layer; i <= end_layer; i++)
								{
									if (cmd[0])
									{
										bindings[i][key1][K_ENABLED].out1 = K_BOUND;
										bindings[i][key1][key2].out1 = keymap[cmd[0]];
										bindings[i][key1][key2].flag1 = 0;
									}
									if (cmd[1]) bindings[i][key1][key2].out2 = keymap[cmd[1]];
									if (cmd[2]) bindings[i][key1][key2].out3 = keymap[cmd[2]];
									if (cmd[3] && cmd[3] != ' ')
									{
										char str[CMD_LEN];
										sprintf(str, "%c", cmd[3]);
										ULONG value = atoi(str);
										bindings[i][key1][key2].arg1 = (USHORT)value;
									}
								}
							}
						}
					}
					memset(cmd, 0, CMD_LEN);

					cmdlen = l = 0;
					key2 = key1 = K_UNDEFINED;
					commenton = FALSE;
				}
				else
				{
					if (!commenton)
					{
						if (c == '"') commenton = TRUE;
						else if (l == 0 && c != ' ') key1 = keymap[c];
						else if (l == 1 && c != ' ') key2 = keymap[c];
						// l == 2, c = ' '
						else if ((key2 == K_UNDEFINED) && (l >= 2)) // single key command
						{
							cmd[l - 2] = c;
							cmdlen++;
						}
						else if ((key2 != K_UNDEFINED) && (l >= 3)) // 2 key command
						{
							cmd[l - 3] = c;
							cmdlen++;
						}
					}
					l++;
				}
			}
		}

		ZwClose(handle);
	}

	loading_config = 2;
	PsTerminateSystemThread(STATUS_SUCCESS);
	return 0;
}


ULONG atoi(char* str)
{
	WCHAR wstr[CMD_LEN];
	UNICODE_STRING ustr;
	ULONG value = 0;

	swprintf(wstr, L"%s", str);
	RtlInitUnicodeString(&ustr, wstr);
	RtlUnicodeStringToInteger(&ustr, 10, &value); // base 10

	return value;
}

VOID Initialize()
{
	// some special characters for system settings
	// settings in the config file overrides the default
	keymap['~'] = K_VARIABLE; // Or for command it means internal command/function call

	keymap[' '] = ' ';

	// Non-obvious keys
	keymap['A'] = K_LALT;
	keymap['B'] = K_BACKSPACE;
	keymap['C'] = K_LCTRL;
	keymap['D'] = K_DEL;
	keymap['E'] = K_ESC;
	keymap['F'] = K_CAPSLOCK;
	keymap['G'] = K_RSHIFT;
	keymap['I'] = K_LSHIFT;
	keymap['H'] = K_LEFT;
	keymap['J'] = K_DOWN;
	keymap['K'] = K_UP;
	keymap['L'] = K_RIGHT;
	keymap['M'] = K_INS;
	keymap['N'] = K_ENTER;
	keymap['O'] = K_HOME;
	keymap['P'] = K_END;
	keymap['Q'] = K_COMMAND;
	keymap['R'] = K_RSHIFT;
	keymap['S'] = K_SPACE;
	keymap['T'] = K_TAB;
	keymap['U'] = K_PGUP;
	keymap['V'] = K_PGDN;
	keymap['W'] = K_LWIN;
	keymap['X'] = K_NUMLOCK;
	keymap['Z'] = K_SCROLLLOCK;
	//keymap[' '] = K_SPACE;
	//keymap['\n'] = K_ENTER;

	// F keys
	keymap['!'] = K_F1;
	keymap['@'] = K_F2;
	keymap['#'] = K_F3;
	keymap['$'] = K_F4;
	keymap['%'] = K_F5;
	keymap['^'] = K_F6;
	keymap['&'] = K_F7;
	keymap['*'] = K_F8;
	keymap['('] = K_F9;
	keymap[')'] = K_F10;
	keymap['_'] = K_F11;
	keymap['+'] = K_F12;


	// alphas
	keymap['a'] = K_A;
	keymap['b'] = K_B;
	keymap['c'] = K_C;
	keymap['d'] = K_D;
	keymap['e'] = K_E;
	keymap['f'] = K_F;
	keymap['g'] = K_G;
	keymap['h'] = K_H;
	keymap['i'] = K_I;
	keymap['j'] = K_J;
	keymap['k'] = K_K;
	keymap['l'] = K_L;
	keymap['m'] = K_M;
	keymap['n'] = K_N;
	keymap['o'] = K_O;
	keymap['p'] = K_P;
	keymap['q'] = K_Q;
	keymap['r'] = K_R;
	keymap['s'] = K_S;
	keymap['t'] = K_T;
	keymap['u'] = K_U;
	keymap['v'] = K_V;
	keymap['w'] = K_W;
	keymap['x'] = K_X;
	keymap['y'] = K_Y;
	keymap['z'] = K_Z;

	// numerics
	keymap['0'] = K_0;
	keymap['1'] = K_1;
	keymap['2'] = K_2;
	keymap['3'] = K_3;
	keymap['4'] = K_4;
	keymap['5'] = K_5;
	keymap['6'] = K_6;
	keymap['7'] = K_7;
	keymap['8'] = K_8;
	keymap['9'] = K_9;

	// punctuations
	keymap['`'] = K_TILDA;
	keymap['-'] = K_MINUS;
	keymap['='] = K_EQUAL;
	keymap['['] = K_SQUARE_L;
	keymap[']'] = K_SQUARE_R;
	keymap['\\'] = K_BACKSLASH;
	keymap[';'] = K_SEMICOLON;
	keymap['\''] = K_QUOTE;
	keymap[','] = K_COMMA;
	keymap['.'] = K_PERIOD;
	keymap['/'] = K_SLASH;

	// default initialization
	layer = 0;
	pause = FALSE;
	key1 = 0;
	key2 = 0;
	key_bind_time = 10; 
	longkey_time = 10; 
	key_bind_timeout = 50;  
	key_repeat_time = 30;
	safe_mode = SETTING_ON;
	capslock_to_lshift = SETTING_OFF;
	reload_config = SETTING_OFF;
	loading_config = 0;
	last_packet_length = 0;

	// hex to dec
	hex2dec['0'] = 0;
	hex2dec['1'] = 1;
	hex2dec['2'] = 2;
	hex2dec['3'] = 3;
	hex2dec['4'] = 4;
	hex2dec['5'] = 5;
	hex2dec['6'] = 6;
	hex2dec['7'] = 7;
	hex2dec['8'] = 8;
	hex2dec['9'] = 9;
	hex2dec['a'] = hex2dec['A'] = 0x0A;
	hex2dec['b'] = hex2dec['B'] = 0x0B;
	hex2dec['c'] = hex2dec['C'] = 0x0C;
	hex2dec['d'] = hex2dec['D'] = 0x0D;
	hex2dec['e'] = hex2dec['E'] = 0x0E;
	hex2dec['f'] = hex2dec['F'] = 0x0F;
}