/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_KMDFDriver1,
    0x5652ba5e,0xa72c,0x4370,0xad,0x0c,0x87,0xd8,0xbe,0xc3,0x5d,0x82);
// {5652ba5e-a72c-4370-ad0c-87d8bec35d82}
