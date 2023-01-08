#include "device.h"
#include "log.h"

NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit)
{
	NTSTATUS                        status;
    WDFDEVICE                       device;    
    WDF_OBJECT_ATTRIBUTES           deviceAttributes;
    WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
    
    UNREFERENCED_PARAMETER(Driver);
	
	debug("Begin EvtDriverDeviceAdd\n");

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

exit1:
    DevCtxHdr->ProfileDrvInterface.BthFreeBrb((PBRB)brb);
exit:
	debug("End RetrieveLocalInfo\n");
    return status;
}

NTSTATUS CreateRequest(WDFDEVICE Device, WDFIOTARGET IoTarget, WDFREQUEST * Request)
{
	NTSTATUS Status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES Attributes;

	debug("Begin CreateRequest\n");

    WDF_OBJECT_ATTRIBUTES_INIT(&Attributes);
    Attributes.ParentObject = Device;

	Status = WdfRequestCreate(&Attributes, IoTarget, Request);
	if(!NT_SUCCESS(Status))
	{
		debug("Failed WdfRequestCreate %d\n", Status);
		goto exit;
	}
	
exit:
	debug("End CreateRequest\n");
	return Status;
}

NTSTATUS PrepareRequest( WDFIOTARGET IoTarget, PBRB BRB, WDFREQUEST Request)
{
	NTSTATUS Status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES MemoryAttributes;
    WDFMEMORY Memory = NULL;

	debug("Begin PrepareRequest\n");

    WDF_OBJECT_ATTRIBUTES_INIT(&MemoryAttributes);
    MemoryAttributes.ParentObject = Request;

	Status = WdfMemoryCreatePreallocated(
        &MemoryAttributes,
        BRB,
		sizeof(*BRB),
        &Memory
        );

	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}
		
	Status = WdfIoTargetFormatRequestForInternalIoctlOthers(
        IoTarget,
        Request,
        IOCTL_INTERNAL_BTH_SUBMIT_BRB,
        Memory, //OtherArg1
        NULL, //OtherArg1Offset
        NULL, //OtherArg2
        NULL, //OtherArg2Offset
        NULL, //OtherArg4
        NULL  //OtherArg4Offset
        );
	
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

exit:
	debug("End PrepareRequest\n");
	return Status;
}

NTSTATUS SendBRB(PWIIMOTE_CONTEXT DeviceContext, WDFREQUEST OptRequest, PBRB BRB, PFN_WDF_REQUEST_COMPLETION_ROUTINE	CompletionRoutine)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;

	debug("Begin SendBRB\n");

	if(OptRequest == NULL)
	{
		Status = CreateRequest(DeviceContext->Header.Device, DeviceContext->Header.IoTarget, &Request);
		if(!NT_SUCCESS(Status))
		{
			goto exit;
		}
	}
	else
	{
		Request = OptRequest;
	}

	Status = PrepareRequest(DeviceContext->Header.IoTarget, BRB, Request);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		goto exit;
	}

	WdfRequestSetCompletionRoutine(
		Request,
		CompletionRoutine,
		BRB
		);

	if(!WdfRequestSend(
		Request,
		DeviceContext->Header.IoTarget,
		WDF_NO_SEND_OPTIONS
		))
	{
        Status = WdfRequestGetStatus(Request);
		WdfObjectDelete(Request);
		goto exit;
	}

exit:
	debug("End SendBRB\n");
	return Status;
}

VOID L2CAPCallback( PVOID Context, INDICATION_CODE Indication, PINDICATION_PARAMETERS Parameters)
{
	debug("Begin L2CAPCallback\n");
	
	//WDF_DEVICE_STATE NewDeviceState;
	PWIIMOTE_CONTEXT DeviceContext = (PWIIMOTE_CONTEXT)Context;

	//UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(Parameters);
	
	debug("L2CAP Channel Callback\n");
	debug("Indication: %u\n", Indication);
	

	if(Indication == IndicationRemoteDisconnect)
	{
		//Wiimote has disconnected.
		//Code has to be added to signal the PnP-Manager that the device is gone.
		
		debug("Disconnect\n");
		debug("Parameter: %u; %u\n", Parameters->Parameters.Disconnect.Reason, Parameters->Parameters.Disconnect.CloseNow);
	
		debug("WiimoteReset\n");
		debug("Signal Device Is Gone\n");

		//WDF_DEVICE_STATE_INIT (&NewDeviceState);

		//HidNotifyPresence(WdfDeviceWdmGetDeviceObject(DeviceContext->Device), FALSE);
		
		//WdfDeviceGetDeviceState(DeviceContext->Device, &NewDeviceState);
		//NewDeviceState.Removed = WdfTrue;
		//WdfDeviceSetDeviceState(DeviceContext->Device, &NewDeviceState);
		//WdfPdoMarkMissing(DeviceContext->Device);
	}
	
	debug("End L2CAPCallback\n");
}

NTSTATUS OpenChannel(PWIIMOTE_CONTEXT DeviceContext, PBRB PreAllocatedBRB, BYTE PSM, PFNBTHPORT_INDICATION_CALLBACK ChannelCallback,PFN_WDF_REQUEST_COMPLETION_ROUTINE ChannelCompletion)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWIIMOTE_DEVICE_CONTEXT_HEADER BluetoothContext = &(DeviceContext->Header);
	PBRB_L2CA_OPEN_CHANNEL BRBOpenChannel;

	debug("Begin OpenChannel\n");

	//Create or reuse BRB
	if(PreAllocatedBRB == NULL)
	{
		BRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)BluetoothContext->ProfileDrvInterface.BthAllocateBrb(BRB_L2CA_OPEN_CHANNEL, POOLTAG_WIIMOTE);
		if (BRBOpenChannel == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			goto exit;
		}
	}
	else
	{
		BluetoothContext->ProfileDrvInterface.BthReuseBrb(PreAllocatedBRB, BRB_L2CA_OPEN_CHANNEL);
		BRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)PreAllocatedBRB;
	}
	
	//Fill BRB
	BRBOpenChannel->BtAddress = 247284104376928;
	BRBOpenChannel->Psm = PSM; //0x13
	BRBOpenChannel->ChannelFlags = 0;
	BRBOpenChannel->ConfigOut.Flags = 0;
    BRBOpenChannel->ConfigOut.Mtu.Max = L2CAP_DEFAULT_MTU;
    BRBOpenChannel->ConfigOut.Mtu.Min = L2CAP_MIN_MTU;
    BRBOpenChannel->ConfigOut.Mtu.Preferred = L2CAP_DEFAULT_MTU;
	BRBOpenChannel->ConfigOut.FlushTO.Max = L2CAP_DEFAULT_FLUSHTO;
	BRBOpenChannel->ConfigOut.FlushTO.Min = L2CAP_MIN_FLUSHTO;
	BRBOpenChannel->ConfigOut.FlushTO.Preferred = L2CAP_DEFAULT_FLUSHTO;
	BRBOpenChannel->ConfigOut.ExtraOptions = 0;
	BRBOpenChannel->ConfigOut.NumExtraOptions = 0;
	BRBOpenChannel->ConfigOut.LinkTO = 0;

    BRBOpenChannel->IncomingQueueDepth = 50;
    BRBOpenChannel->ReferenceObject = (PVOID) WdfDeviceWdmGetDeviceObject(DeviceContext->Header.Device);
   
	if(ChannelCallback != NULL)
	{
		BRBOpenChannel->CallbackFlags = CALLBACK_DISCONNECT;                                                   
		BRBOpenChannel->Callback = ChannelCallback; //L2CAPCallback;
		BRBOpenChannel->CallbackContext = (PVOID)DeviceContext;
	}

	//SendBRB
	Status = SendBRB(DeviceContext, NULL, (PBRB)BRBOpenChannel, ChannelCompletion);
	if(!NT_SUCCESS(Status))
	{
		BluetoothContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRBOpenChannel);
		goto exit;
	}

exit:
	debug("End OpenChannel\n");
	return Status;
}

VOID CleanUpCompletedRequest( WDFREQUEST Request,  WDFIOTARGET IoTarget,  WDFCONTEXT Context)
{
	PWIIMOTE_CONTEXT DeviceContext;
	PWIIMOTE_DEVICE_CONTEXT_HEADER BluetoothContext;
	PBRB UsedBRB;

	debug("Begin CleanUpCompletedRequest\n");

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	BluetoothContext = &(DeviceContext->Header);
	UsedBRB = (PBRB)Context;

	WdfObjectDelete(Request);
	BluetoothContext->ProfileDrvInterface.BthFreeBrb(UsedBRB);
	
	debug("End CleanUpCompletedRequest\n");
}

VOID InterruptChannelCompletion(WDFREQUEST Request, WDFIOTARGET IoTarget, PWDF_REQUEST_COMPLETION_PARAMS Params, WDFCONTEXT Context)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWIIMOTE_CONTEXT DeviceContext;
	PWIIMOTE_DEVICE_CONTEXT_HEADER BluetoothContext;
	PBRB_L2CA_OPEN_CHANNEL UsedBRBOpenChannel;

	debug("Begin InterruptChannelCompletion\n");

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	BluetoothContext = &(DeviceContext->Header);
	UsedBRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)Context;

	Status = Params->IoStatus.Status;
	
	debug("Interrupt Channel Result %08X\n", Status);

	if(!NT_SUCCESS(Status))
	{
		CleanUpCompletedRequest(Request, IoTarget, Context);
		if(Status == STATUS_IO_TIMEOUT)
		{
			debug("Signal Device Is Gone\n");
		}
		else 
		{
			WdfDeviceSetFailed(DeviceContext->Header.Device, WdfDeviceFailedNoRestart);
		}

		return;
	}

	BluetoothContext->InterruptChannelHandle = UsedBRBOpenChannel->ChannelHandle;
	CleanUpCompletedRequest(Request, IoTarget, Context);
	
	// Start Wiimote functionality
	debug("WiimoteStart\n");
	
	debug("End InterruptChannelCompletion\n");
}

VOID ControlChannelCompletion(WDFREQUEST Request, WDFIOTARGET IoTarget, PWDF_REQUEST_COMPLETION_PARAMS Params, WDFCONTEXT Context)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWIIMOTE_CONTEXT DeviceContext;
	PWIIMOTE_DEVICE_CONTEXT_HEADER BluetoothContext;
	PBRB_L2CA_OPEN_CHANNEL UsedBRBOpenChannel;

	debug("Begin ControlChannelCompletion\n");

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	BluetoothContext = &(DeviceContext->Header);
	UsedBRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)Context;

	Status = Params->IoStatus.Status;
	
	debug("Control Channel Result %08X\n", Status);

	if(!NT_SUCCESS(Status))
	{
		CleanUpCompletedRequest(Request, IoTarget, Context);
		if(Status == STATUS_IO_TIMEOUT)
		{
			debug("Signal Device Is Gone\n");
		}
		else 
		{
			WdfDeviceSetFailed(DeviceContext->Header.Device, WdfDeviceFailedNoRestart);
		}

		return;
	}

	BluetoothContext->ControlChannelHandle = UsedBRBOpenChannel->ChannelHandle;
	CleanUpCompletedRequest(Request, IoTarget, Context);
	
	// Open Interrupt Channel
	OpenChannel(DeviceContext, NULL, 0x13, L2CAPCallback, InterruptChannelCompletion);
	
	debug("End ControlChannelCompletion\n");
}

NTSTATUS BluetoothOpenChannels(PWIIMOTE_CONTEXT DeviceContext)
{
	NTSTATUS status;
	
	debug("Begin BluetoothOpenChannels\n");
	
	status = OpenChannel(DeviceContext, NULL, 0x11, NULL, ControlChannelCompletion);
	
	debug("End BluetoothOpenChannels\n");
	
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
	
	status = BluetoothOpenChannels(devCtx);
	if(!NT_SUCCESS(status))
	{
		goto exit;
	}
	
exit:

	debug("End EvtDeviceSelfManagedIoInit\n");
    return status;
}

