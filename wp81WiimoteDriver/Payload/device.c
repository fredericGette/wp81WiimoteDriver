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
		debug("PrepareRequest failed : 0x%08X\n", Status);
		WdfObjectDelete(Request);
		goto exit;
	}

	Status = WdfRequestAllocateTimer(Request);
	if(!NT_SUCCESS(Status))
	{
		debug("WdfRequestAllocateTimer failed : 0x%08X\n", Status);
		WdfObjectDelete(Request);
		goto exit;
	}

	WDF_REQUEST_SEND_OPTIONS_INIT(&SendOptions, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS | WDF_REQUEST_SEND_OPTION_TIMEOUT);
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&SendOptions, SYNCHRONOUS_CALL_TIMEOUT);

	debug("* Before WdfRequestSend\n");
	WdfRequestSend(
		Request,
		DeviceContext->IoTarget,
		&SendOptions
		);
	debug("* After WdfRequestSend\n");
	
	Status = WdfRequestGetStatus(Request);

	if(!NT_SUCCESS(Status))
	{
		debug("WdfRequestGetStatus failed : 0x%08X\n", Status);
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
	Data[0] = 0xA2;	//HID Output Report; WR_SET_REPORT|BT_OUTPUT
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

NTSTATUS RequestStatus(PWIIMOTE_CONTEXT DeviceContext)
{
	CONST size_t BufferSize = 3;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	debug("Begin RequestStatus\n");

	// Get Resources
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x15;	//Status Information Request
	Data[2] = 0x00;	//Rumble Off

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, TRUE);
	if(!NT_SUCCESS(Status))
	{
		goto exit;
	}

exit:
	debug("End RequestStatus\n");
	return Status;
}


NTSTATUS ReadButtons(PWIIMOTE_CONTEXT DeviceContext, PVOID Buffer)
{
	CONST size_t ReadBufferSize = 8;
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
		debug("SendBRB Failed 0x%08X\n", Status);
		WdfObjectDelete(Request);		
		DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRB);
		goto exit;
	}	
	PVOID ReadBuffer2 = BRB->Buffer;
	size_t ReadBufferSize2 = BRB->BufferSize;
	size_t RemainingBufferSize = BRB->RemainingBufferSize;

	debug("BufferSize: %d - RemainingBufferSize: %d\n", ReadBufferSize2, RemainingBufferSize);
	
	if(ReadBufferSize2 >= 2)
	{
		debug("ReadBuffer[0]: 0x%02X input(wiimote->phone)=0xA1 output(phone->wiimote)=0xA2\n", ((BYTE *)ReadBuffer2)[0]);
		debug("ReadBuffer[1]: 0x%02X (ReportID)\n", ((BYTE *)ReadBuffer2)[1]);
		debug("ReadBuffer[2]: 0x%02X\n", ((BYTE *)ReadBuffer2)[2]);
		debug("ReadBuffer[3]: 0x%02X\n", ((BYTE *)ReadBuffer2)[3]);
		((BYTE *)Buffer)[0] = ((BYTE *)ReadBuffer2)[0];
		((BYTE *)Buffer)[1] = ((BYTE *)ReadBuffer2)[1];
		((BYTE *)Buffer)[2] = ((BYTE *)ReadBuffer2)[2];
		((BYTE *)Buffer)[3] = ((BYTE *)ReadBuffer2)[3];
	}
	
	WdfObjectDelete(Request);		
	DeviceContext->ProfileDrvInterface.BthFreeBrb((PBRB)BRB);
	
	if (RemainingBufferSize <= 8)
	{
		// Avoid empty buffer
		RequestStatus(DeviceContext);
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
	
	/////////// control pipe (command channel) ///////////
	
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

    BRBOpenChannel->IncomingQueueDepth = 0;
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

	/////////// data pipe (interrupt channel) ///////////

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

    BRBOpenChannel->IncomingQueueDepth = 0;
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
	WDFDEVICE  device;
	
	debug("Begin EvtIoDeviceControl\n");
	
	device = WdfIoQueueGetDevice(Queue);
	
	PWIIMOTE_CONTEXT devCtx = GetDeviceContext(device);
	
	debug("IOCTL_HID_GET_COLLECTION_INFORMATION=%08X\n",0x000b01a8);
	debug("IOCTL_WIIMOTE_CONNECT=%08X\n",IOCTL_WIIMOTE_CONNECT);
	debug("IOCTL_WIIMOTE_READ=%08X\n",IOCTL_WIIMOTE_READ);
	debug("IoControlCode=%08X\n",IoControlCode);
	
	debug("device=%08X\n",device);
	debug("devCtx=%08X\n",devCtx);
	debug("devCtx->Device=%08X\n",devCtx->Device);
	debug("devCtx->IoTarget=%08X\n",devCtx->IoTarget);
	debug("devCtx->ProfileDrvInterface=%08X\n",devCtx->ProfileDrvInterface);
	
	if (IoControlCode ==  IOCTL_WIIMOTE_CONNECT)
	{
		ConnectWiimote(devCtx);
		WdfRequestComplete(Request, STATUS_SUCCESS);
	}
	else if (IoControlCode == IOCTL_WIIMOTE_READ)
	{
		USHORT  requiredSize = 4;
		PVOID   Buffer;
		size_t  bytesReturned = 0;
		//
        // Get the buffer. Make sure the buffer is big enough
        //
        NTSTATUS status = WdfRequestRetrieveOutputBuffer(
                                                Request, 
                                                (size_t)requiredSize,
                                                &Buffer,
                                                NULL
                                                );
        if(!NT_SUCCESS(status)){
			debug("WdfRequestRetrieveOutputBuffer failed : 0x%08X\n", status);
			WdfRequestComplete(Request, status);
            goto exit;
        }
		
		status = ReadButtons(devCtx, Buffer);
		bytesReturned = 4;
		WdfRequestCompleteWithInformation(
                                      Request,
                                      status,
                                      bytesReturned
                                      );
	}
	else 
	{
		WdfRequestComplete(Request, STATUS_NOT_SUPPORTED);
	}

exit:	
	debug("End EvtIoDeviceControl\n");
}

VOID EvtIoDeviceControlForRawPdo(WDFQUEUE Queue, WDFREQUEST Request, size_t OutputBufferLength, size_t InputBufferLength, ULONG IoControlCode)
{
	NTSTATUS status = STATUS_SUCCESS;
    WDFDEVICE parent = WdfIoQueueGetDevice(Queue);
    PRPDO_DEVICE_DATA pdoData;
    WDF_REQUEST_FORWARD_OPTIONS forwardOptions;
	
	debug("Begin EvtIoDeviceControlForRawPdo\n");
        
    pdoData = PdoGetData(parent);

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

	debug("IoControlCode=%08X\n",IoControlCode);
    //
    // Process the ioctl and complete it when you are done.
    // Since the queue is configured for serial dispatch, you will
    // not receive another ioctl request until you complete this one.
    //
    switch (IoControlCode) {
    case IOCTL_WIIMOTE_CONNECT:
	case IOCTL_WIIMOTE_READ:
        WDF_REQUEST_FORWARD_OPTIONS_INIT(&forwardOptions);
        status = WdfRequestForwardToParentDeviceIoQueue(Request, pdoData->ParentQueue, &forwardOptions);
        if (!NT_SUCCESS(status)) {
            WdfRequestComplete(Request, status);
        }
        break;
    default:
        WdfRequestComplete(Request, status);
        break;
    }

	debug("End EvtIoDeviceControlForRawPdo\n");

    return;
}

NTSTATUS PrepareHardware( WDFDEVICE Device, WDFCMRESLIST ResourcesRaw, WDFCMRESLIST ResourcesTranslated)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(ResourcesRaw);
	UNREFERENCED_PARAMETER(ResourcesTranslated);
	
	
	debug("PrepareHardware\n");
	
	return status;
}

NTSTATUS DeviceD0Entry( WDFDEVICE Device, WDF_POWER_DEVICE_STATE PreviousState)
{
	NTSTATUS status = STATUS_SUCCESS;
	DECLARE_CONST_UNICODE_STRING(symbolicLinkName, SYMBOLIC_NAME_STRING) ;
	
	UNREFERENCED_PARAMETER(PreviousState);
	
	debug("DeviceD0Entry\n");
	
	//
	// Create a symbolic link for the control object so that usermode can open
	// the device.
	//
	debug("WdfDeviceCreateSymbolicLink\n");
	status = WdfDeviceCreateSymbolicLink(Device, &symbolicLinkName);
	if (!NT_SUCCESS(status)) {
		debug("Failed WdfDeviceCreateSymbolicLink %x\n", status);
	}
	
	return status;	
}

NTSTATUS DeviceD0Exit( WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(TargetState);
	
	debug("DeviceD0Exit\n");	

	return status;	
}

NTSTATUS ReleaseHardware( WDFDEVICE Device, WDFCMRESLIST ResourcesTranslated)
{ 
	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(ResourcesTranslated);
	
	debug("ReleaseHardware\n");
	
	return status;	
}

NTSTATUS CreateRawPdo(WDFDEVICE Device)
{
	NTSTATUS                    status;
    PWDFDEVICE_INIT             pDeviceInit = NULL;
    PRPDO_DEVICE_DATA           pdoData = NULL;
    WDFDEVICE                   hChild = NULL;
    WDF_OBJECT_ATTRIBUTES       pdoAttributes;
    WDF_DEVICE_PNP_CAPABILITIES pnpCaps;
    WDF_IO_QUEUE_CONFIG         ioQueueConfig;
    WDFQUEUE                    queue;
    WDF_DEVICE_STATE            deviceState;
    PWIIMOTE_CONTEXT            devExt;
	ULONG InstanceNo = 1; // We need only one PDO.
    DECLARE_CONST_UNICODE_STRING(deviceId,WIIMOTERAWPDO_DEVICE_ID );
    DECLARE_CONST_UNICODE_STRING(hardwareId,WIIMOTERAWPDO_DEVICE_ID );
    DECLARE_CONST_UNICODE_STRING(deviceLocation,L"Wiimote raw PDO\0" );
    DECLARE_UNICODE_STRING_SIZE(buffer, MAX_ID_LEN);
	WDF_PNPPOWER_EVENT_CALLBACKS PnpPowerCallbacks;

    debug("Begin CreateRawPdo\n");

    //
    // Allocate a WDFDEVICE_INIT structure and set the properties
    // so that we can create a device object for the child.
    //
    pDeviceInit = WdfPdoInitAllocate(Device);

    if (pDeviceInit == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    //
    // Mark the device RAW so that the child device can be started
    // and accessed without requiring a function driver. Since we are
    // creating a RAW PDO, we must provide a class guid.
    //
    status = WdfPdoInitAssignRawDevice(pDeviceInit, &GUID_DEVCLASS_BLUETOOTH);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // Assign DeviceID - This will be reported to IRP_MN_QUERY_ID/BusQueryDeviceID
    //
    status = WdfPdoInitAssignDeviceID(pDeviceInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // We could be enumerating more than one children if the filter attaches
    // to multiple instances, so we must provide a
    // BusQueryInstanceID. If we don't, system will throw CA bugcheck.
    //
    status =  RtlUnicodeStringPrintf(&buffer, L"%02d", InstanceNo);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    status = WdfPdoInitAssignInstanceID(pDeviceInit, &buffer);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // Provide a description about the device. This text is usually read from
    // the device. In the case of USB device, this text comes from the string
    // descriptor. This text is displayed momentarily by the PnP manager while
    // it's looking for a matching INF. If it finds one, it uses the Device
    // Description from the INF file to display in the device manager.
    // Since our device is raw device and we don't provide any hardware ID
    // to match with an INF, this text will be displayed in the device manager.
    //
    status = RtlUnicodeStringPrintf(&buffer,L"Wiimote_raw_PDO_%02d", InstanceNo );
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // You can call WdfPdoInitAddDeviceText multiple times, adding device
    // text for multiple locales. When the system displays the text, it
    // chooses the text that matches the current locale, if available.
    // Otherwise it will use the string for the default locale.
    // The driver can specify the driver's default locale by calling
    // WdfPdoInitSetDefaultLocale.
    //
    status = WdfPdoInitAddDeviceText(pDeviceInit,
                                        &buffer,
                                        &deviceLocation,
                                        0x409 // en-US
                                        );
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    WdfPdoInitSetDefaultLocale(pDeviceInit, 0x409);
    
    //
    // Initialize the attributes to specify the size of PDO device extension.
    // All the state information private to the PDO will be tracked here.
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&pdoAttributes, RPDO_DEVICE_DATA);

    //
    // Set up our queue to allow forwarding of requests to the parent
    // This is done so that the cached Wiimote Attributes can be retrieved
    //
    WdfPdoInitAllowForwardingRequestToParent(pDeviceInit);


	// Configure PnP Functions
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&PnpPowerCallbacks);
	PnpPowerCallbacks.EvtDevicePrepareHardware = PrepareHardware;
	PnpPowerCallbacks.EvtDeviceReleaseHardware = ReleaseHardware;
	PnpPowerCallbacks.EvtDeviceD0Entry = DeviceD0Entry;
	PnpPowerCallbacks.EvtDeviceD0Exit = DeviceD0Exit;
	WdfDeviceInitSetPnpPowerEventCallbacks(pDeviceInit, &PnpPowerCallbacks);

    status = WdfDeviceCreate(&pDeviceInit, &pdoAttributes, &hChild);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // Get the device context.
    //
    pdoData = PdoGetData(hChild);

    //
    // Get the parent queue we will be forwarding to
    //
    devExt = GetDeviceContext(Device);
    pdoData->ParentQueue = devExt->rawPdoQueue;

    //
    // Configure the default queue associated with the control device object
    // to be Serial so that request passed to EvtIoDeviceControl are serialized.
    // A default queue gets all the requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching.
    //

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                                    WdfIoQueueDispatchSequential);

    ioQueueConfig.EvtIoDeviceControl = EvtIoDeviceControlForRawPdo;

    status = WdfIoQueueCreate(hChild,
                                        &ioQueueConfig,
                                        WDF_NO_OBJECT_ATTRIBUTES,
                                        &queue // pointer to default queue
                                        );
    if (!NT_SUCCESS(status)) {
        debug("WdfIoQueueCreate failed 0x%x\n", status);
        goto Cleanup;
    }

    //
    // Set some properties for the child device.
    //
    WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);

    pnpCaps.Removable         = WdfTrue;
    pnpCaps.SurpriseRemovalOK = WdfTrue;
    pnpCaps.NoDisplayInUI     = WdfTrue;

    pnpCaps.Address  = InstanceNo;
    pnpCaps.UINumber = InstanceNo;

    WdfDeviceSetPnpCapabilities(hChild, &pnpCaps);

    //
    // TODO: In addition to setting NoDisplayInUI in DeviceCaps, we
    // have to do the following to hide the device. Following call
    // tells the framework to report the device state in
    // IRP_MN_QUERY_DEVICE_STATE request.
    //
    WDF_DEVICE_STATE_INIT(&deviceState);
    deviceState.DontDisplayInUI = WdfTrue;
    WdfDeviceSetDeviceState(hChild, &deviceState);

    //
    // Add this device to the FDO's collection of children.
    // After the child device is added to the static collection successfully,
    // driver must call WdfPdoMarkMissing to get the device deleted. It
    // shouldn't delete the child device directly by calling WdfObjectDelete.
    //
    status = WdfFdoAddStaticChild(Device, hChild);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

	debug("End CreateRawPdo\n");

    //
    // pDeviceInit will be freed by WDF.
    //
    return STATUS_SUCCESS;

Cleanup:

    debug("CreateRawPdo failed %x\n", status);

    //
    // Call WdfDeviceInitFree if you encounter an error while initializing
    // a new framework device object. If you call WdfDeviceInitFree,
    // do not call WdfDeviceCreate.
    //
    if (pDeviceInit != NULL) {
        WdfDeviceInitFree(pDeviceInit);
    }

    if(hChild) {
        WdfObjectDelete(hChild);
    }

    return status;
}

NTSTATUS EvtDriverDeviceAdd(WDFDRIVER  Driver, PWDFDEVICE_INIT  DeviceInit)
{
    UNREFERENCED_PARAMETER(Driver);
	NTSTATUS                        status;
    WDFDEVICE                       device;    
    WDF_OBJECT_ATTRIBUTES           deviceAttributes;
	WDF_IO_QUEUE_CONFIG     ioQueueConfig;
	WDFQUEUE                hQueue;
    
	debug("Begin EvtDriverDeviceAdd\n");

    //
    // Tell the framework that you are filter driver. Framework
    // takes care of inherting all the device flags & characterstics
    // from the lower device you are attaching to.
    //
    WdfFdoInitSetFilter(DeviceInit);

    //
    // Set device attributes
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, WIIMOTE_CONTEXT);
 
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
	
	debug("device=%08X\n",device);
	debug("devCtx=%08X\n",devCtx);
	debug("devCtx->Device=%08X\n",devCtx->Device);
	debug("devCtx->IoTarget=%08X\n",devCtx->IoTarget);
	debug("devCtx->ProfileDrvInterface=%08X\n",devCtx->ProfileDrvInterface);
	
    //
    // Create a new queue to handle IOCTLs that will be forwarded to us from
    // the rawPDO. 
    //
    WDF_IO_QUEUE_CONFIG_INIT(&ioQueueConfig, WdfIoQueueDispatchSequential);

    //
    // Framework by default creates non-power managed queues for
    // filter drivers.
    //
    ioQueueConfig.EvtIoDeviceControl = EvtIoDeviceControl;

    status = WdfIoQueueCreate(device,
                            &ioQueueConfig,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            &hQueue
                            );
    if (!NT_SUCCESS(status)) {
        debug("WdfIoQueueCreate failed 0x%x\n", status);
        goto exit;
    }

	devCtx->rawPdoQueue = hQueue;

    //
    // Create a RAW pdo so we can provide a sideband communication with
    // the application.
    //
    status = CreateRawPdo(device);
			
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
