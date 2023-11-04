// bcdedit /store f:\EFIESP\efi\Microsoft\Boot\BCD /set {default} bootlog Yes
// bcdedit /store f:\EFIESP\efi\Microsoft\Boot\BCD /set {default} testsigning yes
//
// set PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\x86_arm;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin;%PATH%
//
// BthEnum x3	BthLEEnum x1
//      \        /
//      BthMini x3		System\\CurrentControlSet\\Enum\\SystemBusQc\\SMD_BT\\4&315a27b&0&4097
//          |
//     QcBluetooth x2

#include <ntifs.h>
#include <wdf.h>
#include <ntstrsafe.h>

#define WdfFltrTrace(_MSG_) { \
        DbgPrint("Filter!"__FUNCTION__ ": ");   \
        DbgPrint _MSG_;                         \
}

#define REQUIRED_ACCESS_FROM_CTL_CODE(ctrlCode)     (((ULONG)(ctrlCode & 0xC000)) >> 14)

typedef struct _DEVICEFILTER_CONTEXT
{
    //
    // Framework device this context is associated with
    //
    WDFDEVICE Device;

} DEVICEFILTER_CONTEXT, *PDEVICEFILTER_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICEFILTER_CONTEXT, GetDeviceContext);


typedef ULONGLONG BTH_ADDR, *PBTH_ADDR;
#define BTH_MAX_PIN_SIZE            (16)
typedef struct _BTH_PIN_INFO {
	UCHAR pin[BTH_MAX_PIN_SIZE];
	UCHAR pinLength;
} BTH_PIN_INFO, *PBTH_PIN_INFO;
typedef struct _BTH_AUTHENTICATE_RESPONSE {
	ULONG unknown1;
	BTH_ADDR address;
	UCHAR unknown2[520];
	ULONG unknown3;
	BTH_PIN_INFO info;
	ULONG unknown4;
	ULONG unknown5;
	ULONG unknown6;
	ULONG unknown7;
} BTH_AUTHENTICATE_RESPONSE, *PBTH_AUTHENTICATE_RESPONSE;


VOID
FilterForwardRequest(
    IN WDFREQUEST Request,
    IN WDFIOTARGET Target
    )
{
    WDF_REQUEST_SEND_OPTIONS options;
    BOOLEAN ret;
    NTSTATUS status;

    //
    // We are not interested in post processing the IRP so 
    // fire and forget.
    //
    WDF_REQUEST_SEND_OPTIONS_INIT(&options,
                                  WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);

    ret = WdfRequestSend(Request, Target, &options);

    if (ret == FALSE) {
        status = WdfRequestGetStatus (Request);
        DbgPrint("Filter!WdfRequestSend failed: 0x%x\n", status);
        WdfRequestComplete(Request, status);
    }

    return;
}

VOID
FilterEvtIoDeviceControl(
    IN WDFQUEUE      Queue,
    IN WDFREQUEST    Request,
    IN size_t        OutputBufferLength,
    IN size_t        InputBufferLength,
    IN ULONG         IoControlCode
    )
{
	UNREFERENCED_PARAMETER(OutputBufferLength);
    NTSTATUS                        status = STATUS_SUCCESS;
    WDFDEVICE                       device;

    device = WdfIoQueueGetDevice(Queue);
	

	PVOID  buffer;
	size_t  bufSize;
	status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &buffer, &bufSize );
	
	PBTH_AUTHENTICATE_RESPONSE authResponse;
	CHAR pin[BTH_MAX_PIN_SIZE+1];
	RtlZeroMemory(&pin, sizeof(pin));

    switch (IoControlCode) {
		case 0x411004: // IOCTL_BTH_AUTH_RESPONSE
			authResponse = buffer;
			ULONG highAddress = (authResponse->address >> 32);
			ULONG lowAddress = ((authResponse->address << 32) >> 32);
			RtlCopyMemory(pin, authResponse->info.pin, BTH_MAX_PIN_SIZE);
			DbgPrint("Filter!pin=[%s] pin length=%u BT addr=%02X %02X %02X %02X %02X %02X\n", pin, authResponse->info.pinLength, ((highAddress >> 8) & 0xFF), (highAddress & 0xFF), ((lowAddress >> 24) & 0xFF), ((lowAddress >> 16) & 0xFF), ((lowAddress >> 8) & 0xFF), (lowAddress & 0xFF));
			if (strcmp(pin,"---") == 0)
			{
				DbgPrint("Filter!Replace existing pin with computed wiimote pin\n");
				RtlZeroMemory(authResponse->info.pin, BTH_MAX_PIN_SIZE);
				RtlCopyMemory(authResponse->info.pin, &(authResponse->address), 6);
				authResponse->info.pinLength=6;
			}
			break;
    }
    
    if (!NT_SUCCESS(status)) {
        WdfRequestComplete(Request, status);
		goto exit;
        return;
    }

	FilterForwardRequest(Request, WdfDeviceGetIoTarget(device));

exit:

    return;
}


NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit)
{
    UNREFERENCED_PARAMETER(Driver);
	NTSTATUS                        status;
    WDFDEVICE                       device;    
    WDF_OBJECT_ATTRIBUTES           deviceAttributes;
	WDF_IO_QUEUE_CONFIG     		ioQueueConfig;
    
	DbgPrint("Filter!Begin EvtDriverDeviceAdd\n");

    //
    // Tell the framework that you are filter driver. Framework
    // takes care of inherting all the device flags & characterstics
    // from the lower device you are attaching to.
    //
    WdfFdoInitSetFilter(DeviceInit);

    //
    // Set device attributes
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICEFILTER_CONTEXT);
 
	//
    // Create a framework device object.  This call will in turn create
    // a WDM deviceobject, attach to the lower stack and set the
    // appropriate flags and attributes.
    //
    status = WdfDeviceCreate(
        &DeviceInit,
        &deviceAttributes,
        &device
        );
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Filter!WdfDeviceCreate failed with Status code %d\n", status);
        goto exit;
    }
		
				
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchParallel);	
	
	ioQueueConfig.EvtIoDeviceControl = FilterEvtIoDeviceControl;

	status = WdfIoQueueCreate(device,
                            &ioQueueConfig,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            WDF_NO_HANDLE // pointer to default queue
                            );
    if (!NT_SUCCESS(status)) {
        DbgPrint("Filter!WdfIoQueueCreate failed 0x%x\n", status);
        goto exit;
    }   
			
exit:    
	DbgPrint("Filter!End EvtDriverDeviceAdd\n");
    return status;
}

void EvtCleanupCallback(WDFOBJECT DriverObject) 
{
    UNREFERENCED_PARAMETER(DriverObject);
	
	DbgPrint("Filter!Begin EvtCleanupCallback\n");
	DbgPrint("Filter!End EvtCleanupCallback\n");
}

// DriverEntry
NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING  RegistryPath)
{
	DbgPrint("Filter!Begin DriverEntry\n");
	
    NTSTATUS status;
    WDFDRIVER driver;
    WDF_OBJECT_ATTRIBUTES attributes;
        
    WDF_DRIVER_CONFIG DriverConfig;
    WDF_DRIVER_CONFIG_INIT(
                           &DriverConfig,
                           EvtDriverDeviceAdd
                           );

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = EvtCleanupCallback;

    status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        &attributes,
        &DriverConfig,
        &driver
        );

	DbgPrint("Filter!Driver registryPath= %S\n", RegistryPath->Buffer);

	DbgPrint("Filter!End DriverEntry\n");
    return status;
}