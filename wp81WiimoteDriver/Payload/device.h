#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h> 
#include <ntstrsafe.h>
#include <bthdef.h>
#include <ntintsafe.h>
#include <bthguid.h>
#include <bthioctl.h>
#include <sdpnode.h>
#include <bthddi.h>
#include <bthsdpddi.h>
#include <bthsdpdef.h>


#define POOLTAG_WIIMOTE 'wiim'

typedef struct _WIIMOTE_DEVICE_CONTEXT_HEADER
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

    //
    // Local Bluetooth Address
    //
    BTH_ADDR LocalBthAddr;

    //
    // Features supported by the local stack
    //
    BTH_HOST_FEATURE_MASK LocalFeatures;

    //
    // Preallocated request to be reused during initialization/deinitialzation phase
    // Access to this request is not synchronized
    //
    WDFREQUEST Request;
	
	L2CAP_CHANNEL_HANDLE ControlChannelHandle;
	L2CAP_CHANNEL_HANDLE InterruptChannelHandle;
	
} WIIMOTE_DEVICE_CONTEXT_HEADER, *PWIIMOTE_DEVICE_CONTEXT_HEADER;

typedef struct _WIIMOTE_CONTEXT
{
    WIIMOTE_DEVICE_CONTEXT_HEADER  Header;

    //
    // Server address
    //
    BTH_ADDR  ServerBthAddress; 

} WIIMOTE_CONTEXT, *PWIIMOTE_CONTEXT;

//
// Context for requests
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WIIMOTE_CONTEXT, GetDeviceContext);

NTSTATUS EvtDeviceSelfManagedIoInit(WDFDEVICE  Device);
NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit);

NTSTATUS EvtDevicePrepareHardware(WDFDEVICE Device, WDFCMRESLIST ResourceList, WDFCMRESLIST ResourceListTranslated);
NTSTATUS EvtDeviceD0Entry(WDFDEVICE Device, WDF_POWER_DEVICE_STATE  PreviousState);
NTSTATUS EvtDeviceD0Exit(WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState);

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