#include "device.h"
#include "log.h"

NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit)
{
	NTSTATUS                        status;
    WDFDEVICE                       device;    
    WDF_OBJECT_ATTRIBUTES           deviceAttributes;
    WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
    //WDF_FILEOBJECT_CONFIG           fileobjectConfig;
    //WDF_OBJECT_ATTRIBUTES           fileAttributes, requestAttributes;
    //WDF_IO_QUEUE_CONFIG             ioQueueConfig;
    //WDFQUEUE                        queue;
    PWIIMOTE_CONTEXT   devCtx;
    
    UNREFERENCED_PARAMETER(Driver);
	
	debug("Begin EvtDriverDeviceAdd\n");
/*
    //
    // Configure Pnp/power callbacks
    //
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
    pnpPowerCallbacks.EvtDeviceSelfManagedIoInit = EvtDeviceSelfManagedIoInit;

    WdfDeviceInitSetPnpPowerEventCallbacks(
       DeviceInit,
       &pnpPowerCallbacks
       );

    //
    // Configure file callbacks
    //    

    WDF_FILEOBJECT_CONFIG_INIT(
        &fileobjectConfig,
        EvtDeviceFileCreate,
        EvtFileClose,
        WDF_NO_EVENT_CALLBACK // Cleanup
        );

    //
    // Inform framework to create context area in every fileobject
    // so that we can track information per open handle by the
    // application.
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&fileAttributes, WIIMOTE_FILE_CONTEXT);

    WdfDeviceInitSetFileObjectConfig(
        DeviceInit,
        &fileobjectConfig,
        &fileAttributes
        );

    //
    // Inform framework to create context area in every request object.
    //
    // We make BRB as the context since we need BRB for all the requests
    // we handle (Create, Read, Write).
    //

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
        &requestAttributes,
        BRB
        );

    WdfDeviceInitSetRequestAttributes(
        DeviceInit,
        &requestAttributes
        );
*/

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
    pnpPowerCallbacks.EvtDevicePrepareHardware = EvtDevicePrepareHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = EvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = EvtDeviceD0Exit;
	pnpPowerCallbacks.EvtDeviceSelfManagedIoInit = EvtDeviceSelfManagedIoInit;
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    //
    // Set device attributes
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, WIIMOTE_CONTEXT);
 
    status = WdfDeviceCreate(
        &DeviceInit,
        &deviceAttributes,
        &device
        );

    if (!NT_SUCCESS(status))
    {
        debug("WdfDeviceCreate failed with Status code %d\n", status);

        goto exit;
    }

    devCtx = GetDeviceContext(device);
/*
    //
    // Initialize our context
    //
    status = BthEchoCliContextInit(devCtx, device);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP, 
            "Initialization of context failed with Status code %!STATUS!\n", status);

        goto exit;       
    }

    status = BthEchoCliBthQueryInterfaces(devCtx);        
    if (!NT_SUCCESS(status))
    {
        goto exit;
    }

    //
    // Set up our queue
    //

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &ioQueueConfig,
        WdfIoQueueDispatchParallel
        );

    ioQueueConfig.EvtIoRead     = BthEchoCliEvtQueueIoRead;
    ioQueueConfig.EvtIoWrite    = BthEchoCliEvtQueueIoWrite;
    ioQueueConfig.EvtIoStop     = BthEchoCliEvtQueueIoStop;

    status = WdfIoQueueCreate(
        device,
        &ioQueueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &queue
        );
    
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP,
                            "WdfIoQueueCreate failed  %!STATUS!\n", status);
        goto exit;
    }

    //
    // Enable device interface so that app can open a handle to our device
    // and talk to it
    //
    
    status = WdfDeviceCreateDeviceInterface(
        device,
        &BTHECHOSAMPLE_DEVICE_INTERFACE,
        NULL
        );

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, DBG_PNP, 
            "Enabling device interface failed with Status code %!STATUS!\n", status);

        goto exit;       
    }
*/
exit:    
    //
    // We don't need to worry about deleting any objects on failure
    // because all the object created so far are parented to device and when
    // we return an error, framework will delete the device and as a 
    // result all the child objects will get deleted along with that.
    //
	debug("End EvtDriverDeviceAdd\n");
    return status;
}

NTSTATUS EvtDevicePrepareHardware(WDFDEVICE Device, WDFCMRESLIST ResourceList, WDFCMRESLIST ResourceListTranslated)
{
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(ResourceList);
	UNREFERENCED_PARAMETER(ResourceListTranslated);
	debug("Begin EvtDevicePrepareHardware\n");
	debug("End EvtDevicePrepareHardware\n");
	return STATUS_SUCCESS;
}

NTSTATUS EvtDeviceD0Entry(WDFDEVICE Device, WDF_POWER_DEVICE_STATE  PreviousState)
{
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(PreviousState);
	debug("Begin EvtDeviceD0Entry\n");
	debug("End EvtDeviceD0Entry\n");
	return STATUS_SUCCESS;
}

NTSTATUS EvtDeviceD0Exit(WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState)
{
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(TargetState);
	debug("Begin EvtDeviceD0Exit\n");
	debug("End EvtDeviceD0Exit\n");
	return STATUS_SUCCESS;
}

// formats a request with brb and sends it synchronously
NTSTATUS SendBrbSynchronously(WDFIOTARGET IoTarget, WDFREQUEST Request, PBRB Brb, ULONG BrbSize)
{
    NTSTATUS status;
    WDF_REQUEST_REUSE_PARAMS reuseParams;
    WDF_MEMORY_DESCRIPTOR OtherArg1Desc;

	debug("Begin SendBrbSynchronously\n");

    WDF_REQUEST_REUSE_PARAMS_INIT(
        &reuseParams,
        WDF_REQUEST_REUSE_NO_FLAGS, 
        STATUS_NOT_SUPPORTED
        );
	debug("reuseParams=0x%08X\n", reuseParams);

    status = WdfRequestReuse(Request, &reuseParams);
    if (!NT_SUCCESS(status))
    {
		debug("WdfRequestReuse failed, returning status code %d\n", status);        
        goto exit;
    }

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
                        &OtherArg1Desc,
                        Brb,
                        BrbSize
                        );
    
    status = WdfIoTargetSendInternalIoctlOthersSynchronously(
        IoTarget,
        Request,
        IOCTL_INTERNAL_BTH_SUBMIT_BRB,
        &OtherArg1Desc,
        NULL, //OtherArg2
        NULL, //OtherArg4
        NULL, //RequestOptions
        NULL  //BytesReturned
        );

exit:
	debug("End SendBrbSynchronously\n");
    return status;
}

// Retrieves the local bth address.
NTSTATUS RetrieveLocalInfo(PWIIMOTE_DEVICE_CONTEXT_HEADER DevCtxHdr)
{
    NTSTATUS status = STATUS_SUCCESS;
    struct _BRB_GET_LOCAL_BD_ADDR * brb = NULL;
	
	debug("Begin RetrieveLocalInfo\n");
    
    brb = (struct _BRB_GET_LOCAL_BD_ADDR *)DevCtxHdr->ProfileDrvInterface.BthAllocateBrb(BRB_HCI_GET_LOCAL_BD_ADDR, POOLTAG_WIIMOTE);

    if(brb == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;

        debug("Failed to allocate brb BRB_HCI_GET_LOCAL_BD_ADDR, returning status code %d\n", status);        

        goto exit;
    }

    status = SendBrbSynchronously(
        DevCtxHdr->IoTarget,
        DevCtxHdr->Request,
        (PBRB) brb,
        sizeof(*brb)
        );

    if (!NT_SUCCESS(status))
    {
        debug("Retrieving local bth address failed, Status code %d\n", status);        

        goto exit1;        
    }

    DevCtxHdr->LocalBthAddr = brb->BtAddress;
	debug("LocalBthAddr=%012I64X\n", brb->BtAddress);
/*
    //
    // Now retreive local host supported features
    //
    status = BthEchoSharedGetHostSupportedFeatures(DevCtxHdr);
    
    if (!NT_SUCCESS(status))
    {
        debug("Sending IOCTL for reading supported features failed, Status code %d\n", status);

        goto exit1;
    }
*/
exit1:
    DevCtxHdr->ProfileDrvInterface.BthFreeBrb((PBRB)brb);
exit:
	debug("End RetrieveLocalInfo\n");
    return status;
}

// Our one time initialization
NTSTATUS EvtDeviceSelfManagedIoInit(WDFDEVICE  Device)
{
	NTSTATUS status;
	WDF_OBJECT_ATTRIBUTES attributes;
	
	debug("Begin EvtDeviceSelfManagedIoInit\n");
    PWIIMOTE_CONTEXT devCtx = GetDeviceContext(Device);
	debug("devCtx=0x%08X\n",devCtx);
	debug("devCtx->Header=0x%08X\n",&(devCtx->Header));
	debug("&devCtx->Header.Device=0x%08X\n",&(devCtx->Header.Device));
	debug("devCtx->Header.Device=0x%08X\n",devCtx->Header.Device);
	debug("devCtx->Header.IoTarget=0x%08X\n",devCtx->Header.IoTarget);
	debug("&devCtx->Header.Request=0x%08X\n",&(devCtx->Header.Request));
	debug("devCtx->Header.ProfileDrvInterface=0x%08X\n",&(devCtx->Header.ProfileDrvInterface));
	debug("devCtx->Header.ProfileDrvInterface.BthAllocateBrb=0x%08X\n",devCtx->Header.ProfileDrvInterface.BthAllocateBrb);
	
	devCtx->Header.Device = Device;
	devCtx->Header.IoTarget = WdfDeviceGetIoTarget(Device);
	
	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	attributes.ParentObject = Device;
	status = WdfRequestCreate(
		&attributes,
		devCtx->Header.IoTarget,
		&(devCtx->Header.Request)
	);
	if (!NT_SUCCESS(status))
	{
		debug("WdfRequestCreate failed, Status code %d\n", status);        
	}
	
	status = WdfFdoQueryForInterface(
		devCtx->Header.Device,
		&GUID_BTHDDI_PROFILE_DRIVER_INTERFACE,
		(PINTERFACE)(&devCtx->Header.ProfileDrvInterface),
		sizeof(devCtx->Header.ProfileDrvInterface),
		BTHDDI_PROFILE_DRIVER_INTERFACE_VERSION_FOR_QI,
		NULL
	);
	
	if (!NT_SUCCESS(status))
    {
        debug("WdfFdoQueryForInterface failed, Status code %d\n", status);        
    }

	debug("devCtx->Header=0x%08X\n",&(devCtx->Header));
	debug("&devCtx->Header.Device=0x%08X\n",&(devCtx->Header.Device));
	debug("devCtx->Header.Device=0x%08X\n",devCtx->Header.Device);
	debug("Device=0x%08X\n",Device);
	debug("devCtx->Header.IoTarget=0x%08X\n",devCtx->Header.IoTarget);
	debug("devCtx->Header.ProfileDrvInterface=0x%08X\n",&(devCtx->Header.ProfileDrvInterface.BthAllocateBrb));
	debug("devCtx->Header.ProfileDrvInterface.BthAllocateBrb=0x%08X\n",devCtx->Header.ProfileDrvInterface.BthAllocateBrb);

	

    status = RetrieveLocalInfo(&devCtx->Header);
    if (!NT_SUCCESS(status))
    {
        goto exit;
    }
/*
    status = BthEchoCliRetrieveServerBthAddress(devCtx);
    if (!NT_SUCCESS(status))
    {
        goto exit;
    }
*/
exit:
//status=STATUS_SUCCESS;
	debug("End EvtDeviceSelfManagedIoInit\n");
    return status;
}

// Retrive server SDP record.
NTSTATUS RetrieveServerSdpRecord(PWIIMOTE_CONTEXT DevCtx, PBTH_SDP_STREAM_RESPONSE * ServerSdpRecord)
{
    NTSTATUS status, statusReuse, disconnectStatus;
    WDF_MEMORY_DESCRIPTOR inMemDesc;
    WDF_MEMORY_DESCRIPTOR outMemDesc;
    WDF_REQUEST_REUSE_PARAMS ReuseParams;    
    BTH_SDP_CONNECT connect = {0};
    BTH_SDP_DISCONNECT disconnect = {0};
    BTH_SDP_SERVICE_ATTRIBUTE_SEARCH_REQUEST requestSdp = {0};
    BTH_SDP_STREAM_RESPONSE responseSdp = {0};
    ULONG requestSize;
    PBTH_SDP_STREAM_RESPONSE serverSdpRecord = NULL;
    WDFREQUEST request;
    WDF_OBJECT_ATTRIBUTES attributes;
    
	debug("Begin RetrieveServerSdpRecord\n");

    //
    // Allocate the request we will use for obtaining sdp record
    // NOTE that we do it for every file open, hence we
    //
    // can't use reserve request from the context
    //
    
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

    status = WdfRequestCreate(
        &attributes,
        DevCtx->Header.IoTarget,
        &request
        );

    if (!NT_SUCCESS(status))
    {
        debug("Failed to allocate request for retriving server sdp record, Status code %d\n", status);

        goto exit;        
    }

    connect.bthAddress = DevCtx->ServerBthAddress;
    connect.requestTimeout = SDP_REQUEST_TO_DEFAULT;
    connect.fSdpConnect = 0;

    //
    // Connect to the SDP service.
    //

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &inMemDesc,
        &connect,
        sizeof(connect)
        );
    
    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &outMemDesc,
        &connect,
        sizeof(connect)
        );

    status = WdfIoTargetSendIoctlSynchronously(
        DevCtx->Header.IoTarget,
        request,
        IOCTL_BTH_SDP_CONNECT,
        &inMemDesc,
        &outMemDesc,
        NULL,   //sendOptions
        NULL    //bytesReturned
        );

    if (!NT_SUCCESS(status))
    {
		debug("IOCTL_BTH_SDP_CONNECT failed, Status code %d\n", status);

        goto exit1;
    }

    //
    // Obtain the required size of the SDP record
    //
    requestSdp.hConnection = connect.hConnection;
    //requestSdp.uuids[0].u.uuid128 = BTHECHOSAMPLE_SVC_GUID;
    requestSdp.uuids[0].uuidType = SDP_ST_UUID128;
    requestSdp.range[0].minAttribute = 0;
    requestSdp.range[0].maxAttribute = 0xFFFF;

    WDF_REQUEST_REUSE_PARAMS_INIT(&ReuseParams, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_NOT_SUPPORTED);
    statusReuse = WdfRequestReuse(request, &ReuseParams);    
    NT_ASSERT(NT_SUCCESS(statusReuse));
    UNREFERENCED_PARAMETER(statusReuse);

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &inMemDesc,
        &requestSdp,
        sizeof(requestSdp)
        );
    
    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &outMemDesc,
        &responseSdp,
        sizeof(responseSdp)
        );

    status = WdfIoTargetSendIoctlSynchronously(
        DevCtx->Header.IoTarget,
        request,
        IOCTL_BTH_SDP_SERVICE_ATTRIBUTE_SEARCH,
        &inMemDesc,
        &outMemDesc,
        NULL,   //sendOptions
        NULL    //bytesReturned
        );

    if (!NT_SUCCESS(status))
    {
        debug("IOCTL_BTH_SDP_SERVICE_ATTRIBUTE_SEARCH failed while querying response size, status code %d\n", status);

        goto exit2;
    }

    //
    // Allocate the required size for SDP record
    //

    status = RtlULongAdd(
        responseSdp.requiredSize, 
        sizeof(BTH_SDP_STREAM_RESPONSE), 
        &requestSize
        );

    if(!NT_SUCCESS(status))
    {
        debug("SDP record size too large, status code %d\n", status);

        goto exit2;
    }

    serverSdpRecord = ExAllocatePoolWithTag(NonPagedPool, requestSize, POOLTAG_WIIMOTE);
    if (NULL == serverSdpRecord)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        debug("Allocating SDP record failed, returning status code %d\n", status); 

        goto exit2;
    }

    //
    // Send request with required size
    //
    
    WDF_REQUEST_REUSE_PARAMS_INIT(&ReuseParams, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_NOT_SUPPORTED);
    statusReuse = WdfRequestReuse(request, &ReuseParams);    
    NT_ASSERT(NT_SUCCESS(statusReuse));
    UNREFERENCED_PARAMETER(statusReuse);

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &inMemDesc,
        &requestSdp,
        sizeof(requestSdp)
        );
    
    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &outMemDesc,
        serverSdpRecord,
        requestSize
        );

    status = WdfIoTargetSendIoctlSynchronously(
        DevCtx->Header.IoTarget,
        request,
        IOCTL_BTH_SDP_SERVICE_ATTRIBUTE_SEARCH,
        &inMemDesc,
        &outMemDesc,
        NULL,   //sendOptions
        NULL    //bytesReturned
        );

    if (!NT_SUCCESS(status))
    {
        debug("IOCTL_BTH_SDP_SERVICE_ATTRIBUTE_SEARCH failed, status code %d\n", status);

        ExFreePoolWithTag(serverSdpRecord, POOLTAG_WIIMOTE);
    }
    else
    {
        *ServerSdpRecord = serverSdpRecord;
    }
    
exit2:
    
    //
    // Disconnect from SDP service.
    //
    
    WDF_REQUEST_REUSE_PARAMS_INIT(&ReuseParams, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_NOT_SUPPORTED);
    statusReuse = WdfRequestReuse(request, &ReuseParams);    
    NT_ASSERT(NT_SUCCESS(statusReuse));
    UNREFERENCED_PARAMETER(statusReuse);

    disconnect.hConnection = connect.hConnection;

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &inMemDesc,
        &disconnect,
        sizeof(disconnect)
        );
    
    disconnectStatus = WdfIoTargetSendIoctlSynchronously(
        DevCtx->Header.IoTarget,
        request,
        IOCTL_BTH_SDP_DISCONNECT,
        &inMemDesc,
        NULL,   //outMemDesc
        NULL,   //sendOptions
        NULL    //bytesReturned
        );

    NT_ASSERT(NT_SUCCESS(disconnectStatus)); //Disconnect should not fail

    if (!NT_SUCCESS(disconnectStatus))
    {
        debug("IOCTL_BTH_SDP_DISCONNECT failed, Status code %d\n", status);
    }
    
exit1:    
    WdfObjectDelete(request);
exit:
	debug("End RetrieveServerSdpRecord\n");
    return status;
}

// Retrieve PSM from the SDP record
NTSTATUS RetrievePsmFromSdpRecord(PBTHDDI_SDP_PARSE_INTERFACE SdpParseInterface, PBTH_SDP_STREAM_RESPONSE ServerSdpRecord, USHORT * Psm)
{
    NTSTATUS    status = STATUS_SUCCESS;
    PUCHAR      nextElement;
    ULONG       nextElementSize;

	debug("Begin RetrievePsmFromSdpRecord\n");

    PSDP_TREE_ROOT_NODE sdpTree = NULL;
    PSDP_NODE           nodeProtoDescList = NULL;
    PSDP_NODE           nodeProto0 = NULL;
    PSDP_NODE           nodeProto0UUID = NULL;
    PSDP_NODE           nodeProto0SParam0 = NULL;

    SdpParseInterface->SdpGetNextElement(
         &(ServerSdpRecord->response[0]),
         ServerSdpRecord->responseSize,
         NULL,
         &nextElement,
         &nextElementSize
         );
    
    if(nextElementSize == 0)
    {
        status = STATUS_DEVICE_DATA_ERROR;
		debug("Getting first element from SDP record failed, returning status code %d\n", status);
        goto exit;
    }

    status = SdpParseInterface->SdpConvertStreamToTree(
        nextElement,
        nextElementSize,
        &sdpTree,
        POOLTAG_WIIMOTE
        );
    
    if(!NT_SUCCESS(status))
    {
        debug("Converting SDP record to tree failed, status code %d\n", status);
        goto exit;
    }

    //
    //Find PROTOCOL_DESCRIPTOR_LIST in the tree
    //
    status = SdpParseInterface->SdpFindAttributeInTree(
        sdpTree,
        (USHORT)SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST,
        &nodeProtoDescList
        );
    
    if(!NT_SUCCESS(status))
    {
        debug("FindAttribute failed for SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST, status code %d\n", status);
        goto exit0;
    }

    if(nodeProtoDescList->hdr.Type != SDP_TYPE_SEQUENCE)
    {
        goto SdpFormatError;
    }

    //
    // Get the next sequence.
    //
    
    if(nodeProtoDescList->u.sequence.Link.Flink == NULL)
    {
        goto SdpFormatError;
    }

    nodeProto0 = CONTAINING_RECORD(nodeProtoDescList->u.sequence.Link.Flink, SDP_NODE, hdr.Link);

    if(nodeProto0->hdr.Type != SDP_TYPE_SEQUENCE)
    {
        goto SdpFormatError;
    }

    if(nodeProto0->u.sequence.Link.Flink == NULL)
    {
        goto SdpFormatError;
    }

    //
    // Get the first GUID, (L2CAP)
    //
    
    nodeProto0UUID = CONTAINING_RECORD(nodeProto0->u.sequence.Link.Flink, SDP_NODE, hdr.Link);

    if(nodeProto0UUID->hdr.Type != SDP_TYPE_UUID)
    {
        goto SdpFormatError;
    }
    
    if(nodeProto0UUID->hdr.Link.Flink == NULL)
    {
        goto SdpFormatError;
    }

    //
    // Get the PSM
    //
    
    nodeProto0SParam0 = CONTAINING_RECORD(nodeProto0UUID->hdr.Link.Flink, SDP_NODE, hdr.Link);
    if(nodeProto0SParam0->hdr.SpecificType != SDP_ST_UINT16)
    {
        goto SdpFormatError;
    }
        
    *Psm = nodeProto0SParam0->u.uint16;

    debug("Psm: %d", *Psm);
    
    goto exit0;

SdpFormatError:

    status = STATUS_DEVICE_DATA_ERROR;

    debug("Parsing error due to invalid SDP record, returning status code %d\n", status);
    
exit0:

    SdpParseInterface->SdpFreeTree(sdpTree);

exit:    
	debug("End RetrievePsmFromSdpRecord\n");
    return status;    
}


// an application opens a handle to our device.
VOID EvtDeviceFileCreate(WDFDEVICE  Device, WDFREQUEST  Request, WDFFILEOBJECT  FileObject)
{
    NTSTATUS status;
    PBTH_SDP_STREAM_RESPONSE serverSdpRecord = NULL;
    PWIIMOTE_CONTEXT devCtx = GetDeviceContext(Device);
    PWIIMOTE_FILE_CONTEXT fileCtx = GetFileContext(FileObject);
    BTHDDI_SDP_PARSE_INTERFACE sdpParseInterface;
        
	debug("Begin EvtDeviceFileCreate\n");	
		
    //
    // We need to obtained PSM from the server
    // For this, first we retrieve the SDP record
    //
    
    status = RetrieveServerSdpRecord(devCtx, &serverSdpRecord);
    if (!NT_SUCCESS(status))
    {
        goto exit;
    }

    status = WdfFdoQueryForInterface(
        Device,
        &GUID_BTHDDI_SDP_PARSE_INTERFACE, 
        (PINTERFACE) (&sdpParseInterface),
        sizeof(sdpParseInterface), 
        BTHDDI_SDP_PARSE_INTERFACE_VERSION_FOR_QI, 
        NULL
        );
                
    if (!NT_SUCCESS(status))
    {
        debug("QueryInterface failed for Interface sdp parse interface, version %d, Status code %d\n",BTHDDI_SDP_PARSE_INTERFACE_VERSION_FOR_QI, status);
        goto exit;
    }

    //
    // Once we retrieved the server SDP record, we retrieve
    // PSM from this record.
    //
    // We store this Psm in our file context and use it in open
    // and close channel BRBs.
    //

    status = RetrievePsmFromSdpRecord(
        &sdpParseInterface,
        serverSdpRecord,
        &fileCtx->ServerPsm
        );
    
    if (!NT_SUCCESS(status))
    {
        goto exit;
    }

exit:
    if (NULL != serverSdpRecord)
    {
        //
        // Free the Server SDP record
        //
        ExFreePoolWithTag(serverSdpRecord, POOLTAG_WIIMOTE);
    }    

    //
    // If we failed we complete the request here
    // If it succeeds RemoteConnectCompletion will complete the request
    //

    if (!NT_SUCCESS(status))
    {
        WdfRequestComplete(Request, status);        
    }
	
	debug("End EvtDeviceFileCreate\n");	
}

// I/O manager sends Close IRP for a file.
VOID EvtFileClose(WDFFILEOBJECT  FileObject)
{    
	UNREFERENCED_PARAMETER(FileObject);
	debug("Begin EvtFileClose\n");	
	debug("End EvtFileClose\n");	
}
