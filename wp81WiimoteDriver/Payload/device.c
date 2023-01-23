#include "device.h"

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

NTSTATUS CreateBuffer( WDFREQUEST Request, SIZE_T BufferSize, WDFMEMORY * Memory, PVOID * Buffer)
{
	NTSTATUS Status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES Attributes;
	
	debug("Begin CreateBuffer\n");

	WDF_OBJECT_ATTRIBUTES_INIT(&Attributes);
	Attributes.ParentObject = Request;
	
	Status = WdfMemoryCreate(&Attributes, NonPagedPool, POOLTAG_WIIMOTE, BufferSize, Memory, Buffer);
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

exit:
	debug("End CreateBuffer\n");
	return Status;
}

NTSTATUS BluetoothCreateRequestAndBuffer(WDFDEVICE Device, WDFIOTARGET IoTarget, SIZE_T BufferSize, WDFREQUEST * Request, WDFMEMORY * Memory, PVOID * Buffer)
{
	NTSTATUS Status = STATUS_SUCCESS;
	
	debug("Begin BluetoothCreateRequestAndBuffer\n");

	Status = CreateRequest(Device, IoTarget, Request);
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

	Status = CreateBuffer((*Request), BufferSize, Memory, Buffer);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(*Request);
		(*Request) = NULL;
		goto exit;
	}
	
exit:	
	debug("End BluetoothCreateRequestAndBuffer\n");
	return Status;
}

NTSTATUS SendBRB(PWIIMOTE_CONTEXT DeviceContext, WDFREQUEST OptRequest, PBRB BRB, PFN_WDF_REQUEST_COMPLETION_ROUTINE CompletionRoutine)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;

	debug("Begin SendBRB\n");

	if(OptRequest == NULL)
	{
		Status = CreateRequest(DeviceContext->Device, DeviceContext->IoTarget, &Request);
		if(!NT_SUCCESS(Status))
		{
			goto exit;
		}
	}
	else
	{
		Request = OptRequest;
	}

	Status = PrepareRequest(DeviceContext->IoTarget, BRB, Request);
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
		DeviceContext->IoTarget,
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

NTSTATUS SendBRBSynchronous(PWIIMOTE_CONTEXT DeviceContext, WDFREQUEST OptRequest, PBRB BRB)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDF_REQUEST_SEND_OPTIONS SendOptions;
	WDFREQUEST Request;

	debug("Begin SendBRBSynchronous\n");

	if(OptRequest == NULL)
	{
		Status = CreateRequest(DeviceContext->Device, DeviceContext->IoTarget, &Request);
		if(!NT_SUCCESS(Status))
		{
			goto exit;
		}
	}
	else
	{
		Request = OptRequest;
	}

	Status = PrepareRequest(DeviceContext->IoTarget, BRB, Request);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		goto exit;
	}

	Status = WdfRequestAllocateTimer(Request);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		goto exit;
	}

	WDF_REQUEST_SEND_OPTIONS_INIT(&SendOptions, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS | WDF_REQUEST_SEND_OPTION_TIMEOUT);
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&SendOptions, SYNCHRONOUS_CALL_TIMEOUT);

	WdfRequestSend(
		Request,
		DeviceContext->IoTarget,
		&SendOptions
		);
	
	Status = WdfRequestGetStatus(Request);

	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		goto exit;
	}

exit:
	debug("End SendBRBSynchronous\n");
	return Status;
}

VOID CleanUpCompletedRequest( WDFREQUEST Request,  WDFIOTARGET IoTarget,  WDFCONTEXT Context)
{
	PWIIMOTE_CONTEXT DeviceContext;
	PBRB UsedBRB;

	debug("Begin CleanUpCompletedRequest\n");

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	UsedBRB = (PBRB)Context;

	WdfObjectDelete(Request);
	DeviceContext->ProfileDrvInterface.BthFreeBrb(UsedBRB);
	
	debug("End CleanUpCompletedRequest\n");
}

VOID TransferToDeviceCompletion(WDFREQUEST Request, WDFIOTARGET IoTarget,PWDF_REQUEST_COMPLETION_PARAMS Params, WDFCONTEXT Context)
{
	UNREFERENCED_PARAMETER(Params);
	debug("Begin TransferToDeviceCompletion\n");

	CleanUpCompletedRequest(Request, IoTarget, Context);
	debug("End TransferToDeviceCompletion\n");
}

NTSTATUS BluetoothTransferToDevice(PWIIMOTE_CONTEXT DeviceContext, WDFREQUEST Request, WDFMEMORY Memory, BOOLEAN Synchronous)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PBRB_L2CA_ACL_TRANSFER BRBTransfer;
	size_t BufferSize;
	
	debug("Begin BluetoothTransferToDevice\n");
	
	if(DeviceContext->InterruptChannelHandle == NULL)
	{
		debug("InterruptChannelHandle is null.\n");
		Status = STATUS_INVALID_HANDLE;
		goto exit;
	}

	// Now get an BRB and fill it
	BRBTransfer = (PBRB_L2CA_ACL_TRANSFER)DeviceContext->ProfileDrvInterface.BthAllocateBrb(BRB_L2CA_ACL_TRANSFER, POOLTAG_WIIMOTE);
	if (BRBTransfer == NULL)
	{
		debug("BthAllocateBrb failed\n");
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}

	BRBTransfer->BtAddress = 247284104376928;
	BRBTransfer->ChannelHandle = DeviceContext->InterruptChannelHandle;
	BRBTransfer->TransferFlags = ACL_TRANSFER_DIRECTION_OUT;
	BRBTransfer->BufferMDL = NULL;
	BRBTransfer->Buffer = WdfMemoryGetBuffer(Memory, &BufferSize);
	BRBTransfer->BufferSize = (ULONG)BufferSize;

	//Send
	if(Synchronous)
	{
		Status = SendBRBSynchronous(DeviceContext, Request, (PBRB)BRBTransfer);
		WdfObjectDelete(Request);
		DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRBTransfer);
		if(!NT_SUCCESS(Status))
		{
			debug("BthFreeBrb failed : 0x%08X\n", Status);
			goto exit;
		}
	}
	else
	{
		Status = SendBRB(DeviceContext, Request, (PBRB)BRBTransfer, TransferToDeviceCompletion);	
		if(!NT_SUCCESS(Status))
		{
			DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRBTransfer);
			goto exit;
		}
	}

exit:
	debug("End BluetoothTransferToDevice\n");
	return Status;
}

VOID L2CAPCallback(PVOID Context, INDICATION_CODE Indication, PINDICATION_PARAMETERS Parameters)
{
	debug("Begin L2CAPCallback\n");
	
	PWIIMOTE_CONTEXT DeviceContext = (PWIIMOTE_CONTEXT)Context;

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
	}
	
	debug("End L2CAPCallback\n");
}


NTSTATUS SetLEDs(PWIIMOTE_CONTEXT DeviceContext, BYTE LEDFlag)
{
	CONST size_t BufferSize = 3;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	debug("Begin SetLEDs\n");
	
	// Get Resources
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x11;	//Player LED
	Data[2] = LEDFlag;

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, TRUE);
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

exit:
	debug("End SetLEDs\n");
	return Status;
}

NTSTATUS SetReportMode(PWIIMOTE_CONTEXT DeviceContext, BYTE ReportMode)
{
	CONST size_t BufferSize = 4;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	debug("Begin SetReportMode\n");

	// Get Resources
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x12;	//Set ReportMode
	Data[2] = 0x00;	//Only On Change
	Data[3] = ReportMode; //Mode

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, TRUE);
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

exit:
	debug("End SetReportMode\n");
	return Status;
}

NTSTATUS ReadButtons(PWIIMOTE_CONTEXT DeviceContext)
{
	CONST size_t ReadBufferSize = 50;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	PBRB_L2CA_ACL_TRANSFER BRB;
	PVOID ReadBuffer = NULL;

	debug("Begin ReadButtons\n");

	//Create Report And Buffer
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, ReadBufferSize, &Request, &Memory, &ReadBuffer);
	if(!NT_SUCCESS(Status))
	{
		debug("CreateRequestAndBuffer Failed 0x%08X", Status);
		goto exit;
	}

	// Create BRB
	BRB = (PBRB_L2CA_ACL_TRANSFER)DeviceContext->ProfileDrvInterface.BthAllocateBrb(BRB_L2CA_ACL_TRANSFER, POOLTAG_WIIMOTE);
	if (BRB == NULL)
	{
		debug("BthAllocateBrb STATUS_INSUFFICIENT_RESOURCES\n");
		WdfObjectDelete(Request);
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}

	// Read
	BRB->BtAddress = 247284104376928;
	BRB->ChannelHandle = DeviceContext->InterruptChannelHandle;
	BRB->TransferFlags = ACL_TRANSFER_DIRECTION_IN | ACL_SHORT_TRANSFER_OK;
	BRB->BufferMDL = NULL;
	BRB->Buffer = ReadBuffer;
	BRB->BufferSize = (ULONG)ReadBufferSize;

	Status = SendBRBSynchronous(DeviceContext, Request, (PBRB)BRB);
	if(!NT_SUCCESS(Status))
	{
		debug("SendBRB Failed 0x%08X", Status);
		goto exit;
	}	
	PVOID ReadBuffer2 = BRB->Buffer;
	size_t ReadBufferSize2 = BRB->BufferSize;

	debug("RawBuffer: %I64X\n", (UINT64 * )ReadBuffer2);
	debug("BufferSize: %d - RemainingBufferSize: %d\n", BRB->BufferSize, BRB->RemainingBufferSize);
	
	if(ReadBufferSize2 >= 2)
	{
		debug("ReadBuffer[0]: 0x%02X input(wiimote->phone)=0xA1 output(phone->wiimote)=0xA2\n", ((BYTE *)ReadBuffer2)[0]);
		debug("ReadBuffer[1]: 0x%02X (ReportID)\n", ((BYTE *)ReadBuffer2)[1]);
		debug("ReadBuffer[2]: 0x%02X\n", ((BYTE *)ReadBuffer2)[2]);
		debug("ReadBuffer[3]: 0x%02X\n", ((BYTE *)ReadBuffer2)[3]);
	}

exit:
	debug("End ReadButtons\n");
	return Status;
}

NTSTATUS ConnectWiimote(PWIIMOTE_CONTEXT DeviceContext)
{
	NTSTATUS Status;
	WDFREQUEST Request;
	PBRB_L2CA_OPEN_CHANNEL BRBOpenChannel;
	
	debug("Begin ConnectWiimote\n");
	
	/////////// control pipe ///////////
	
	//Create BRB
	BRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)DeviceContext->ProfileDrvInterface.BthAllocateBrb(BRB_L2CA_OPEN_CHANNEL, POOLTAG_WIIMOTE);
	if (BRBOpenChannel == NULL)
	{
		debug("BthAllocateBrb Failed\n");
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}
	
	//Fill BRB
	BRBOpenChannel->BtAddress = 247284104376928;
	BRBOpenChannel->Psm = 0x11;
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
    BRBOpenChannel->ReferenceObject = (PVOID) WdfDeviceWdmGetDeviceObject(DeviceContext->Device);
   
	Status = CreateRequest(DeviceContext->Device, DeviceContext->IoTarget, &Request);
	if(!NT_SUCCESS(Status))
	{
		debug("CreateRequest Failed 0x%08X\n", Status);
		goto exit;
	}
   
	//SendBRB
	Status = SendBRBSynchronous(DeviceContext, Request, (PBRB)BRBOpenChannel);
	if(!NT_SUCCESS(Status))
	{
		debug("SendBRB Failed 0x%08X\n", Status);
		debug("Control Channel Result %08X (C00000B5 = timeout; C00000D0=max connection reached)\n", Status);
		
		if(Status == STATUS_IO_TIMEOUT)
		{
			debug("Signal Device Is Gone\n");
		}

		WdfObjectDelete(Request);		
		DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRBOpenChannel);
		goto exit;
	}	

	DeviceContext->ControlChannelHandle = BRBOpenChannel->ChannelHandle;
	WdfObjectDelete(Request);		
	DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRBOpenChannel);

	/////////// data pipe ///////////

	//Create BRB
	BRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)DeviceContext->ProfileDrvInterface.BthAllocateBrb(BRB_L2CA_OPEN_CHANNEL, POOLTAG_WIIMOTE);
	if (BRBOpenChannel == NULL)
	{
		debug("BthAllocateBrb Failed\n");
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}
	
	//Fill BRB
	BRBOpenChannel->BtAddress = 247284104376928;
	BRBOpenChannel->Psm = 0x13;
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
    BRBOpenChannel->ReferenceObject = (PVOID) WdfDeviceWdmGetDeviceObject(DeviceContext->Device);
   
	BRBOpenChannel->CallbackFlags = CALLBACK_DISCONNECT;                                                   
	BRBOpenChannel->Callback = L2CAPCallback;
	BRBOpenChannel->CallbackContext = (PVOID)DeviceContext;
   
	Status = CreateRequest(DeviceContext->Device, DeviceContext->IoTarget, &Request);
	if(!NT_SUCCESS(Status))
	{
		debug("CreateRequest Failed 0x%08X\n", Status);
		goto exit;
	}
   
	//SendBRB
	Status = SendBRBSynchronous(DeviceContext, Request, (PBRB)BRBOpenChannel);
	if(!NT_SUCCESS(Status))
	{
		debug("SendBRB Failed 0x%08X\n", Status);
		debug("Control Channel Result %08X (C00000B5 = timeout; C00000D0=max connection reached)\n", Status);
		
		if(Status == STATUS_IO_TIMEOUT)
		{
			debug("Signal Device Is Gone\n");
		}

		WdfObjectDelete(Request);		
		DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRBOpenChannel);
		goto exit;
	}	

	DeviceContext->InterruptChannelHandle = BRBOpenChannel->ChannelHandle;
	WdfObjectDelete(Request);		
	DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRBOpenChannel);
	
	// Start Wiimote functionality
	debug("WiimoteStart\n");
	SetLEDs(DeviceContext, WIIMOTE_LEDS_ONE);
	SetReportMode(DeviceContext, 0x30);

	
exit:
	debug("End ConnectWiimote\n");
	return Status;
}


void EvtIoDeviceControl(WDFQUEUE Queue, WDFREQUEST Request, size_t OutputBufferLength, size_t InputBufferLength, ULONG IoControlCode)
{
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);
	WDFDEVICE  Device;
	
	debug("Begin EvtIoDeviceControl\n");
	
	Device = WdfIoQueueGetDevice(Queue);
	PWIIMOTE_CONTEXT devCtx = GetDeviceContext(Device);
	
	debug("IOCTL_HID_GET_COLLECTION_INFORMATION=%08X\n",0x000b01a8);
	debug("IOCTL_WIIMOTE_CONNECT=%08X\n",IOCTL_WIIMOTE_CONNECT);
	debug("IOCTL_WIIMOTE_READ=%08X\n",IOCTL_WIIMOTE_READ);
	debug("IoControlCode=%08X\n",IoControlCode);
	if (IoControlCode ==  IOCTL_WIIMOTE_CONNECT)
	{
		ConnectWiimote(devCtx);
		WdfRequestComplete(Request, STATUS_SUCCESS);
	}
	else if (IoControlCode == IOCTL_WIIMOTE_READ)
	{
		ReadButtons(devCtx);
		WdfRequestComplete(Request, STATUS_SUCCESS);
	}
	else 
	{
		WdfRequestComplete(Request, STATUS_NOT_SUPPORTED);
	}
	
	debug("End EvtIoDeviceControl\n");
}

NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit)
{
    UNREFERENCED_PARAMETER(Driver);
	NTSTATUS                        status;
    WDFDEVICE                       device;    
    WDF_OBJECT_ATTRIBUTES           deviceAttributes;
	WDF_IO_QUEUE_CONFIG 			QueueConfig;
	WDFQUEUE                    	queue;
	WDFSTRING 						string;
	UNICODE_STRING 					unicodeString;
    
	debug("Begin EvtDriverDeviceAdd\n");

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

	PWIIMOTE_CONTEXT devCtx = GetDeviceContext(device);
	devCtx->Device = device;
	devCtx->IoTarget = WdfDeviceGetIoTarget(device);
	
	status = WdfFdoQueryForInterface(
		device,
		&GUID_BTHDDI_PROFILE_DRIVER_INTERFACE,
		(PINTERFACE)(&devCtx->ProfileDrvInterface),
		sizeof(devCtx->ProfileDrvInterface),
		BTHDDI_PROFILE_DRIVER_INTERFACE_VERSION_FOR_QI,
		NULL
	);
	if (!NT_SUCCESS(status))
    {
        debug("WdfFdoQueryForInterface failed, Status code %d\n", status);  
		goto exit;		
    }
	
	
    // Create an interface so that usermode can open the device.
    
	debug("WdfDeviceCreateDeviceInterface\n");
	status = WdfDeviceCreateDeviceInterface(device, (LPGUID) &GUID_DEVINTERFACE_HID, NULL);
	if(!NT_SUCCESS(status))
	{
		debug("WdfDeviceCreateDeviceInterface failed with Status code %d\n", status);
        goto exit;
	}
	
	debug("WdfStringCreate\n");
	status = WdfStringCreate(NULL, WDF_NO_OBJECT_ATTRIBUTES, &string);
	if (NT_SUCCESS(status)) {
		debug("WdfDeviceRetrieveDeviceInterfaceString\n");
		status = WdfDeviceRetrieveDeviceInterfaceString(device, &GUID_DEVINTERFACE_HID, NULL, string);
		if (!NT_SUCCESS(status)) {
			debug("WdfDeviceRetrieveDeviceInterfaceString failed with Status code %d\n", status);
			goto exit;
		}
	}
	
	debug("WdfStringGetUnicodeString\n");
	WdfStringGetUnicodeString(string, &unicodeString);
	debug("DeviceInterfaceString : %wZ\n", &unicodeString);
	
	// Create Queue for IOCTL
	WDF_IO_QUEUE_CONFIG_INIT(&QueueConfig, WdfIoQueueDispatchSequential);
	QueueConfig.EvtIoDeviceControl = EvtIoDeviceControl;
	
	debug("WdfIoQueueCreate\n");
	status = WdfIoQueueCreate(device, &QueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
	if(!NT_SUCCESS(status))
	{
		debug("WdfIoQueueCreate failed with Status code %d\n", status);
        goto exit;
	}
	
	debug("WdfDeviceConfigureRequestDispatching\n");
	status = WdfDeviceConfigureRequestDispatching(device, queue, WdfRequestTypeDeviceControl);
	if(!NT_SUCCESS(status))
	{
		debug("WdfDeviceConfigureRequestDispatching failed with Status code %d\n", status);
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
