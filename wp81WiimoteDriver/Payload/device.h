#pragma once

#include <ntifs.h>
#include <wdf.h>
#include <bthdef.h>
#include <bthioctl.h>
#include <bthddi.h>
#include <initguid.h> 
#include <bthguid.h>
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

} WIIMOTE_CONTEXT, *PWIIMOTE_CONTEXT;

//
// Context for requests
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WIIMOTE_CONTEXT, GetDeviceContext);

NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit);

typedef struct _BRB_L2CA_OPEN_CHANNEL * PBRB_L2CA_OPEN_CHANNEL;
typedef struct _BRB_L2CA_CLOSE_CHANNEL * PBRB_L2CA_CLOSE_CHANNEL;
typedef struct _BRB_L2CA_ACL_TRANSFER * PBRB_L2CA_ACL_TRANSFER;

#define SYNCHRONOUS_CALL_TIMEOUT (-1000000000) // 1 Second

// LEDs
#define WIIMOTE_LEDS_ONE (0x10)
#define WIIMOTE_LEDS_TWO (0x20)
#define WIIMOTE_LEDS_THREE (0x40)
#define WIIMOTE_LEDS_FOUR (0x80)
#define WIIMOTE_LEDS_ALL (WIIMOTE_LEDS_FOUR | WIIMOTE_LEDS_THREE | WIIMOTE_LEDS_TWO | WIIMOTE_LEDS_ONE)