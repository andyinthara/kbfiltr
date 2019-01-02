/*++
Copyright (c) 1997  Microsoft Corporation

Module Name:

    kbfilter.h

Abstract:

    This module contains the common private declarations for the keyboard
    packet filter

Environment:

    kernel mode only

--*/

#ifndef KBFILTER_H
#define KBFILTER_H

#pragma warning(disable:4201)

#include "ntddk.h"
#include "kbdmou.h"
#include "ntddkbd.h"
#include "ntdd8042.h"

#pragma warning(default:4201)

#include "wdf.h"

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

#include <initguid.h>
#include <devguid.h>

#include "public.h"

#define KBFILTER_POOL_TAG (ULONG) 'tlfK'

#if DBG

#define TRAP()                      DbgBreakPoint()

#define DebugPrint(_x_) DbgPrint _x_

#else   // DBG

#define TRAP()

#define DebugPrint(_x_)

#endif

#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))

typedef struct _DEVICE_EXTENSION
{
    WDFDEVICE WdfDevice;

    //
    // Queue for handling requests that come from the rawPdo
    //
    WDFQUEUE rawPdoQueue;

    //
    // Number of creates sent down
    //
    LONG EnableCount;

    //
    // The real connect data that this driver reports to
    //
    CONNECT_DATA UpperConnectData;

    //
    // Previous initialization and hook routines (and context)
    //
    PVOID UpperContext;
    PI8042_KEYBOARD_INITIALIZATION_ROUTINE UpperInitializationRoutine;
    PI8042_KEYBOARD_ISR UpperIsrHook;

    //
    // Write function from within KbFilter_IsrHook
    //
    IN PI8042_ISR_WRITE_PORT IsrWritePort;

    //
    // Queue the current packet (ie the one passed into KbFilter_IsrHook)
    //
    IN PI8042_QUEUE_PACKET QueueKeyboardPacket;

    //
    // Context for IsrWritePort, QueueKeyboardPacket
    //
    IN PVOID CallContext;

    //
    // Cached Keyboard Attributes
    //
    KEYBOARD_ATTRIBUTES KeyboardAttributes;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, FilterGetData)


typedef struct _WORKER_ITEM_CONTEXT {

    WDFREQUEST  Request;
    WDFIOTARGET IoTarget;

} WORKER_ITEM_CONTEXT, *PWORKER_ITEM_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WORKER_ITEM_CONTEXT, GetWorkItemContext)

//
// Prototypes
//
DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD KbFilter_EvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL KbFilter_EvtIoDeviceControlForRawPdo;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL KbFilter_EvtIoDeviceControlFromRawPdo;
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL KbFilter_EvtIoInternalDeviceControl;

NTSTATUS
KbFilter_InitializationRoutine(
    IN PVOID                           InitializationContext,
    IN PVOID                           SynchFuncContext,
    IN PI8042_SYNCH_READ_PORT          ReadPort,
    IN PI8042_SYNCH_WRITE_PORT         WritePort,
    OUT PBOOLEAN                       TurnTranslationOn
    );

BOOLEAN
KbFilter_IsrHook(
    PVOID                  IsrContext,
    PKEYBOARD_INPUT_DATA   CurrentInput,
    POUTPUT_PACKET         CurrentOutput,
    UCHAR                  StatusByte,
    PUCHAR                 DataByte,
    PBOOLEAN               ContinueProcessing,
    PKEYBOARD_SCAN_STATE   ScanState
    );

VOID
KbFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

EVT_WDF_REQUEST_COMPLETION_ROUTINE
KbFilterRequestCompletionRoutine;


//
// IOCTL Related defintions
//

//
// Used to identify kbfilter bus. This guid is used as the enumeration string
// for the device id.
DEFINE_GUID(GUID_BUS_KBFILTER,
0xa65c87f9, 0xbe02, 0x4ed9, 0x92, 0xec, 0x1, 0x2d, 0x41, 0x61, 0x69, 0xfa);
// {A65C87F9-BE02-4ed9-92EC-012D416169FA}

DEFINE_GUID(GUID_DEVINTERFACE_KBFILTER,
0x3fb7299d, 0x6847, 0x4490, 0xb0, 0xc9, 0x99, 0xe0, 0x98, 0x6a, 0xb8, 0x86);
// {3FB7299D-6847-4490-B0C9-99E0986AB886}


#define  KBFILTR_DEVICE_ID L"{A65C87F9-BE02-4ed9-92EC-012D416169FA}\\KeyboardFilter\0"


typedef struct _RPDO_DEVICE_DATA
{

    ULONG InstanceNo;

    //
    // Queue of the parent device we will forward requests to
    //
    WDFQUEUE ParentQueue;

} RPDO_DEVICE_DATA, *PRPDO_DEVICE_DATA;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RPDO_DEVICE_DATA, PdoGetData)


NTSTATUS
KbFiltr_CreateRawPdo(
    WDFDEVICE       Device,
    ULONG           InstanceNo
);

#define MAX_KB_INPUT_DATA	64	
#define MAX_KEYOUT			128	
#define MAX_LAYERS			5
#define MAX_KEYS			256	
#define CMD_LEN				16	
#define MSG_LEN				260	
#define CONFIG_BUFFER_SIZE	10000
#define PHRASE_LEN			256
#define PHRASE_MAX			10	
#define COMMAND_LEN			16	


typedef struct _keydata {
	USHORT key;
	USHORT flag;
} keydata;

keydata Keydata(USHORT k, USHORT f);

// keybindings 2D matrix
typedef struct binding {
	USHORT out1;
	USHORT out2;
	USHORT out3;
	USHORT flag1;
	USHORT arg1;
} struct_binding;


USHORT hex2dec[MAX_KEYS];
struct_binding bindings[MAX_LAYERS][MAX_KEYS][MAX_KEYS];
USHORT keymap[MAX_KEYS];
char phrases[PHRASE_MAX][PHRASE_LEN];
char commands[MAX_KEYS][COMMAND_LEN];
USHORT key1, key2, outputed;
#define ALL_LAYERS				255	
USHORT layer;
USHORT pause;
USHORT loading_config;
USHORT config_loaded;

// settings, parameters
#define SETTING_ON				1
#define SETTING_OFF				0
LONGLONG khold, krelease;
ULONG key_bind_time;
ULONG longkey_time;
ULONG key_repeat_time;
ULONG key_bind_timeout;
ULONG safe_mode;
ULONG ignore_capslock;
ULONG capslock_to_lshift;
ULONG reload_config;
ULONG last_packet_length;
CHAR buffer[CONFIG_BUFFER_SIZE];
keydata keyout[MAX_KEYOUT];
ULONG kcount;

#define KEY_MODE_BINDING_ON		1
#define KEY_MODE_BINDING_OFF	2
#define KEY_MODE_LOAD_CONFIG	0
#define KEY_MODE_KEYBOARD_OFF	0
#define KEY_PRESSED				0x00
#define KEY_RELEASED			0x01
#define KEY_NOTSET				0

VOID Initialize();
VOID ThreadLoadConfig();
INT LoadConfig(); // LPCWSTR filename);
VOID Command(USHORT cmd, USHORT key1, USHORT key2, struct_binding binding);
VOID Keyoutput(USHORT layer, USHORT key1, USHORT key2);

// Standard keycodes
/*
00 is normally an error code
01 (Esc)
02 (1!), 03 (2@), 04 (3#), 05 (4$), 06 (5%E), 07 (6^), 08 (7&), 09 (8*), 0a (9(), 0b (0)), 0c (-_), 0d (=+), 0e (Backspace)
0f (Tab), 10 (Q), 11 (W), 12 (E), 13 (R), 14 (T), 15 (Y), 16 (U), 17 (I), 18 (O), 19 (P), 1a ([{), 1b (]})
1c (Enter) 1d (LCtrl)
1e (A), 1f (S), 20 (D), 21 (F), 22 (G), 23 (H), 24 (J), 25 (K), 26 (L), 27 (;:), 28 ('")
29 (`~)
2a (LShift)
2b (\|), on a 102-key keyboard
2c (Z), 2d (X), 2e (C), 2f (V), 30 (B), 31 (N), 32 (M), 33 (,<), 34 (.>), 35 (/?), 36 (RShift)
37 (Keypad-*) or (* /PrtScn) on a 83 / 84 - key keyboard
38 (LAlt), 39 (Space bar),
3a (CapsLock)
3b (F1), 3c (F2), 3d (F3), 3e (F4), 3f (F5), 40 (F6), 41 (F7), 42 (F8), 43 (F9), 44 (F10)
45 (NumLock)
46 (ScrollLock)
47 (Keypad - 7 / Home), 48 (Keypad - 8 / Up), 49 (Keypad - 9 / PgUp)
4a (Keypad--)
4b (Keypad - 4 / Left), 4c (Keypad - 5), 4d (Keypad - 6 / Right), 4e (Keypad - +)
4f (Keypad - 1 / End), 50 (Keypad - 2 / Down), 51 (Keypad - 3 / PgDn)
52 (Keypad - 0 / Ins), 53 (Keypad - . / Del)
55 is less common; occurs e.g.as F11 on a Cherry G80 - 0777 keyboard, as F12 on a Telerate keyboard, as PF1 on a Focus 9000 keyboard, and as FN on an IBM ThinkPad.
5B (L windows)

KEYBOARD_INPUT_DATA
----------------------
UnitId
Specifies the unit number of a keyboard device. A keyboard device name has the format \Device\KeyboardPortN, 
where the suffix N is the unit number of the device. For example, a device, whose name is \Device\KeyboardPort0, has a unit number of zero, and a device, whose name is \Device\KeyboardPort1, has a unit number of one.

MakeCode
Specifies the scan code associated with a key press.

Flags
Specifies a bitwise OR of one or more of the following flags that indicate whether a key was pressed or released, 
and other miscellaneous information.

Value		Meaning
KEY_MAKE	The key was pressed.
KEY_BREAK	The key was released.
KEY_E0		Extended scan code used to indicate special keyboard functions.
KEY_E1		Extended scan code used to indicate special keyboard functions.

*/
// ROW 1

#define K_BOUND			0x01
#define K_ENABLED		0x00
#define K_UNDEFINED		0x00
#define K_SINGLE		0xF2

#define K_ERROR			0x00
#define K_ESC			0x01
#define K_1				0x02
#define K_2				0x03
#define K_3				0x04
#define K_4				0x05
#define K_5				0x06
#define K_6				0x07
#define K_7				0x08
#define K_8				0x09
#define K_9				0x0a
#define K_0				0x0b
#define K_MINUS			0x0c
#define K_EQUAL			0x0d
#define K_BACKSPACE		0x0e

// ROW 2
#define K_TAB			0x0f
#define K_Q				0x10
#define K_W				0x11
#define K_E				0x12
#define K_R				0x13
#define K_T				0x14
#define K_Y				0x15
#define K_U				0x16
#define K_I				0x17
#define K_O				0x18
#define K_P				0x19
#define K_SQUARE_L		0x1a
#define K_SQUARE_R		0x1b
#define K_ENTER			0x1c
#define K_LCTRL			0x1d

// ROW 3
#define K_A				0x1e
#define K_S				0x1f
#define K_D				0x20
#define K_F				0x21
#define K_G				0x22
#define K_H				0x23
#define K_J				0x24
#define K_K				0x25
#define K_L				0x26
#define K_SEMICOLON		0x27
#define K_QUOTE			0x28
#define K_TILDA			0x29
#define K_LSHIFT		0x2a
#define K_BACKSLASH		0x2b

// ROW 4
#define K_Z				0x2c
#define K_X				0x2d
#define K_C				0x2e
#define K_V				0x2f
#define K_B				0x30
#define K_N				0x31
#define K_M				0x32
#define K_COMMA			0x33
#define K_PERIOD		0x34
#define K_SLASH			0x35
#define K_RSHIFT		0x36

#define K_KP_MULT			0x37
#define K_LALT				0x38
#define K_SPACE				0x39
#define K_CAPSLOCK			0x3a
#define K_F1				0x3b
#define K_F2				0x3c
#define K_F3				0x3d
#define K_F4				0x3e
#define K_F5				0x3f
#define K_F6				0x40
#define K_F7				0x41
#define K_F8				0x42
#define K_F9				0x43
#define K_F10				0x44
#define K_NUMLOCK			0x45
#define K_SCROLLLOCK		0x46

#define K_HOME				0x47
#define K_UP				0x48
#define K_PGUP				0x49
#define K_KP_MINUS			0x4a
#define K_LEFT				0x4b
#define K_KP_5				0x4c
#define K_RIGHT				0x4d
#define K_KP_PLUS			0x4e
#define K_END				0x4f
#define K_DOWN				0x50
#define K_PGDN				0x51
#define K_INS				0x52
#define K_DEL				0x53

#define K_F11				0x54
#define K_F12				0x55

#define K_LWIN				0x5B
#define K_RWIN				0x5C

#define K_VARIABLE			0xF0
#define K_COMMAND			0xF1


ULONG atoi(char*);

#endif  // KBFILTER_H

/*

Scan Code               Key       Scan Code               Key         Scan Code               Key
  Set    Set  Set  USB              Set    Set  Set  USB                Set    Set  Set  USB
   1      2    3                     1      2    3                       1      2    3

   01     76   08   29  Esc          37     7C            * PrtSc     E0 5E  E0 37            Power
   02     16   16   1E  ! 1          37+    7C+  7E   55  * KP        E0 5F  E0 3F            Sleep
   03     1E   1E   1F  @ 2       37/54+ 7C/84   57   46  PrtSc       E0 63  E0 5E            Wake
   04     26   26   20  # 3          38     11   19   E2  Alt L       E0 20  E0 23        7F  Mute
   05     25   25   21  $ 4       E0 38  E0 11   39   E6  Alt R       E0 30  E0 33        80  Volume Up
   06     2E   2E   22  % 5          39     29   29   2C  Space       E0 2E  E0 21        81  Volume Down
   07     36   36   23  ^ 6          3A     58   14   39  Caps Lock   E0 17  E0 43        7B  Cut
   08     3D   3D   24  & 7          3B     05   07   3A  F1          E0 18  E0 44        7C  Copy
   09     3E   3E   25  * 8          3C     06   0F   3B  F2          E0 0A  E0 46        7D  Paste
   0A     46   46   26  ( 9          3D     04   17   3C  F3          E0 3B  E0 05        75  Help
   0B     45   45   27  ) 0          3E     0C   1F   3D  F4          E0 08  E0 3D        7A  Undo
   0C     4E   4E   2D  _ -          3F     03   27   3E  F5          E0 07  E0 36            Redo
   0D     55   55   2E  + =          40     0B   2F   3F  F6          E0 22  E0 34            Play
   0E     66   66   2A  Back Space   41     83   37   40  F7          E0 24  E0 3B            Stop
   0F     0D   0D   2B  Tab          42     0A   3F   41  F8          E0 10  E0 15            Skip Back
   10     15   15   14  Q            43     01   47   42  F9          E0 19  E0 4D            Skip Fwd
   11     1D   1D   1A  W            44     09   4F   43  F10         E0 2C  E0 1A            Eject
   12     24   24   08  E            45+    77+  76   53  Num Lock    E0 1E  E0 1C            Mail
   13     2D   2D   15  R         45/46+ 77/7E+  62   48  Pause/Bk    E0 32  E0 3A            Web
   14     2C   2C   17  T            46     7E            ScrLk/Bk    E0 3C  E0 06            Music
   15     35   35   1C  Y            46+    7E+  5F   47  Scroll Lock E0 64  E0 08            Pictures
   16     3C   3C   18  U            47     6C   6C   5F  7 Home KP   E0 6D  E0 50            Video
   17     43   43   0C  I         E0 47* E0 6C*  6E   4A  Home CP
   18     44   44   12  O            48     75   75   60  8 Up KP        5B     1F   08   68  F13
   19     4D   4D   13  P         E0 48* E0 75*  63   52  Up CP          5C     27   10   69  F14
   1A     54   54   2F  { [          49     7D   7D   61  9 PgUp KP      5D     2F   18   6A  F15
   1B     5B   5B   30  } ]       E0 49* E0 7D*  6F   4B  PgUp CP        63     5E   2C   6B  F16
   1C     5A   5A   28  Enter        4A     7B   84   56  - KP           64     08   2B   6C  F17
E0 1C  E0 5A   79   58  Enter KP     4B     6B   6B   5C  4 Left KP      65     10   30   6D  F18
   1D     14   11   E0  Ctrl L    E0 4B* E0 6B*  61   50  Left CP        66     18   38   6E  F19
E0 1D  E0 14   58   E4  Ctrl R       4C     73   73   97  5 KP           67     20   40   6F  F20
   1E     1C   1C   04  A            4D     74   74   5E  6 Right KP     68     28   48   70  F21     
   1F     1B   1B   16  S         E0 4D* E0 74*  6A   4F  Right CP       69     30   50   71  F22
   20     23   23   07  D            4E     79   7C   57  + KP           6A     38   57   72  F23
   21     2B   2B   09  F            4F     69   69   59  1 End KP       6B     40   5F   73  F24
   22     34   34   0A  G         E0 4F* E0 69*  65   4D  End CP                          75              Help
   23     33   33   0B  H            50     72   72   5A  2 Down KP     [71]    19   05   9A  Attn  SysRq
   24     3B   3B   0D  J         E0 50* E0 72*  60   51  Down CP        76     5F   06   9C  Clear
   25     42   42   0E  K            51     7A   7A   5B  3 PgDn KP                       76              Stop
   26     4B   4B   0F  L         E0 51* E0 7A*  6D   4E  PgDn CP                         77              Again
   27     4C   4C   33  : ;          52     70   70   62  0 Ins KP       72     39   04   A3  CrSel       Properties
   28     52   52   34  " '       E0 52* E0 70*  67   49  Ins CP                     0C       Pause ErInp
   29     0E   0E   35  ~ `          53     71   71   63  . Del KP                        78              Undo
   2A     12   12   E1  Shift L   E0 53* E0 71*  64   4C  Del CP         74     53   03   A4  ExSel SetUp
   2B     5D   5C   31  | \          54     84            SysRq          6D     50   0E       ErEOF Recrd
   2B     5D   53   53  (INT 2)      56     61   13   64  (INT 1)        
   2C     1A   1A   1D  Z            57     78   56   44  F11                             80              Copy
   2D     22   22   1B  X            58     07   5E   45  F12                        83       Print Ident
   2E     21   21   06  C         E0 5B  E0 1F   8B   E3  Win L          6F     6F   0A       Copy  Test
   2F     2A   2A   19  V         E0 5C  E0 27   8C   E7  Win R          
   30     32   32   05  B         E0 5D  E0 2F   8D   65  WinMenu                         81              Paste
   31     31   31   11  N            70     13   87   88  katakana       75     5C   01       Enl   Help
   32     3A   3A   10  M            73     51   51   87  (INT 3)        6C     48   09       Ctrl
   33     41   41   36  < ,          77     62        8C  furigana                        82              Find
   34     49   49   37  > .          79     64   86   8A  kanji                           79              Cut
   35     4A   4A   38  ? /          7B     67   85   8B  hiragana
   35+    4A+  77   54  / KP         7D     6A   5D   89  (INT 4)     E0 4C  E0 73   62       Rule
   36     59   59   E5  Shift R     [7E]    6D   7B       (INT 5)

   ---     ---------------   ---------------   ---------------   -----------
   | 01|   | 3B| 3C| 3D| 3E| | 3F| 40| 41| 42| | 43| 44| 57| 58| |+37|+46|+45|
   ---     ---------------   ---------------   ---------------   -----------

   -----------------------------------------------------------   -----------   ---------------
   | 29| 02| 03| 04| 05| 06| 07| 08| 09| 0A| 0B| 0C| 0D|     0E| |*52|*47|*49| |+45|+35|+37| 4A|
   |-----------------------------------------------------------| |-----------| |---------------|
   |   0F| 10| 11| 12| 13| 14| 15| 16| 17| 18| 19| 1A| 1B|   2B| |*53|*4F|*51| | 47| 48| 49|   |
   |-----------------------------------------------------------|  -----------  |-----------| 4E|
   |    3A| 1E| 1F| 20| 21| 22| 23| 24| 25| 26| 27| 28|      1C|               | 4B| 4C| 4D|   |
   |-----------------------------------------------------------|      ---      |---------------|
   |      2A| 2C| 2D| 2E| 2F| 30| 31| 32| 33| 34| 35|        36|     |*4C|     | 4F| 50| 51|   |
   |-----------------------------------------------------------|  -----------  |-----------|-1C|
   |   1D|-5B|   38|                       39|-38|-5C|-5D|  -1D| |*4B|*50|*4D| |     52| 53|   |
   -----------------------------------------------------------   -----------   ---------------

   http://www.quadibloc.com/comp/scan.htm
   https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

*/

