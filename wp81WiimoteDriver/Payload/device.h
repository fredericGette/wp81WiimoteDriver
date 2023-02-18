#pragma once

#include <ntifs.h>
#include <wdf.h>
#include <bthdef.h>
#include <bthioctl.h>
#include <bthddi.h>
#include <initguid.h> 
#include <bthguid.h>
#include <Hidclass.h>
#include "log.h"


#define POOLTAG_WIIMOTE 'wiim'

typedef struct _WIIMOTE_CONTEXT
{
    //
    // Framework device this context is associated with
    //
    WDFDEVICE Device;

    //
    // Default I/O target
    //
    WDFIOTARGET IoTarget;

    //
    // Profile driver interface which contains profile driver DDI
    //
    BTH_PROFILE_DRIVER_INTERFACE ProfileDrvInterface;
	
	L2CAP_CHANNEL_HANDLE ControlChannelHandle;
	L2CAP_CHANNEL_HANDLE InterruptChannelHandle;
	
	//
    // Queue for handling requests that come from the rawPdo
    //
    WDFQUEUE rawPdoQueue;

} WIIMOTE_CONTEXT, *PWIIMOTE_CONTEXT;

//
// Context for requests
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WIIMOTE_CONTEXT, GetDeviceContext);

NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit);

typedef struct _BRB_L2CA_OPEN_CHANNEL * PBRB_L2CA_OPEN_CHANNEL;
typedef struct _BRB_L2CA_CLOSE_CHANNEL * PBRB_L2CA_CLOSE_CHANNEL;
typedef struct _BRB_L2CA_ACL_TRANSFER * PBRB_L2CA_ACL_TRANSFER;

#define WIIMOTE_DEVICE 0x8000

#define IOCTL_WIIMOTE_CONNECT CTL_CODE(WIIMOTE_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_WIIMOTE_READ CTL_CODE(WIIMOTE_DEVICE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define SYNCHRONOUS_CALL_TIMEOUT (-1000000000) // 1 Second

// LEDs
#define WIIMOTE_LEDS_ONE (0x10)
#define WIIMOTE_LEDS_TWO (0x20)
#define WIIMOTE_LEDS_THREE (0x40)
#define WIIMOTE_LEDS_FOUR (0x80)
#define WIIMOTE_LEDS_ALL (WIIMOTE_LEDS_FOUR | WIIMOTE_LEDS_THREE | WIIMOTE_LEDS_TWO | WIIMOTE_LEDS_ONE)


#define  WIIMOTERAWPDO_DEVICE_ID L"{9cb351aa-8523-4f88-8567-f3a60d285ca0}\\WiimoteRawPdo\0"
//#define  KBFILTR_DEVICE_ID L"{A65C87F9-BE02-4ed9-92EC-012D416169FA}\\KeyboardFilter\0"
#define MAX_ID_LEN 128

//{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}
DEFINE_GUID(GUID_DEVCLASS_BLUETOOTH, 0xE0CBF06C, 0xCD8B, 0x4647, 0xBB, 0x8A, 0x26, 0x3B, 0x43, 0xF0, 0xF9, 0x74);

typedef struct _RPDO_DEVICE_DATA
{

    //
    // Queue of the parent device we will forward requests to
    //
    WDFQUEUE ParentQueue;

} RPDO_DEVICE_DATA, *PRPDO_DEVICE_DATA;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RPDO_DEVICE_DATA, PdoGetData);

#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\WiimoteRawPdo"