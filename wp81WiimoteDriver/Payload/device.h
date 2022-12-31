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

typedef enum _WIIMOTE_CONNECTION_STATE {
    ConnectionStateUnitialized = 0,
    ConnectionStateInitialized,        
    ConnectionStateConnecting,
    ConnectionStateConnected,
    ConnectionStateConnectFailed,
    ConnectionStateDisconnecting,
    ConnectionStateDisconnected
    
} WIIMOTE_CONNECTION_STATE, *PWIIMOTE_CONNECTION_STATE;

typedef struct _WIIMOTE_CONNECTION {

    //
    // List entry for connection list maintained at device level
    //
    LIST_ENTRY                              ConnectionListEntry;

    PWIIMOTE_DEVICE_CONTEXT_HEADER    		DevCtxHdr;    

    WIIMOTE_CONNECTION_STATE          		ConnectionState;
    
    //
    // Connection lock, used to synchronize access to _BTHECHO_CONNECTION data structure
    //
    WDFSPINLOCK                             ConnectionLock;

    USHORT                                  OutMTU;
    USHORT                                  InMTU;

    L2CAP_CHANNEL_HANDLE                    ChannelHandle;
    BTH_ADDR                                RemoteAddress;

    //
    // Preallocated Brb, Request used for connect/disconnect
    //
    struct _BRB                             ConnectDisconnectBrb;
    WDFREQUEST                              ConnectDisconnectRequest;

    //
    // Event used to wait for disconnection
    // It is non-signaled when connection is in ConnectionStateDisconnecting
    // transitionary state and signaled otherwise
    //
    KEVENT                                  DisconnectEvent;

    //
    // Continuous readers (used only by server)
    // PLEASE NOTE that KMDF USB Pipe Target uses a single continuous reader
    //
    //BTHECHO_CONTINUOUS_READER               ContinuousReader;
} WIIMOTE_CONNECTION, *PWIIMOTE_CONNECTION;

typedef struct _WIIMOTE_FILE_CONTEXT
{
    //
    // Connection to server opened for this file
    //
    PWIIMOTE_CONNECTION Connection;

    //
    // Server PSM
    //
    USHORT  ServerPsm;    
} WIIMOTE_FILE_CONTEXT, *PWIIMOTE_FILE_CONTEXT;

//
// Context for file objects
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WIIMOTE_FILE_CONTEXT, GetFileContext);

//
// Context for requests
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(BRB, GetRequestContext);

NTSTATUS EvtDeviceSelfManagedIoInit(WDFDEVICE  Device);
NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit);
VOID EvtDeviceFileCreate(WDFDEVICE  Device, WDFREQUEST  Request, WDFFILEOBJECT  FileObject);
VOID EvtFileClose(WDFFILEOBJECT  FileObject);
NTSTATUS RetrievePsmFromSdpRecord(PBTHDDI_SDP_PARSE_INTERFACE SdpParseInterface, PBTH_SDP_STREAM_RESPONSE ServerSdpRecord, USHORT * Psm);

NTSTATUS EvtDevicePrepareHardware(WDFDEVICE Device, WDFCMRESLIST ResourceList, WDFCMRESLIST ResourceListTranslated);
NTSTATUS EvtDeviceD0Entry(WDFDEVICE Device, WDF_POWER_DEVICE_STATE  PreviousState);
NTSTATUS EvtDeviceD0Exit(WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState);