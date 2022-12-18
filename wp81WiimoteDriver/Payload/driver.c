// bcdedit /store f:\EFIESP\efi\Microsoft\Boot\BCD /set {default} bootlog Yes
// bcdedit /store f:\EFIESP\efi\Microsoft\Boot\BCD /set {default} testsigning yes
//
// set PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\x86_arm;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin;%PATH%
//
// CL.exe /c /I"C:\Program Files (x86)\Windows Phone Kits\8.1\Include\km" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\Shared" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\km" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\wdf\kmdf\1.11" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\km\crt" /Zi /W4 /WX /Od /D _ARM_ /D ARM /D _USE_DECLSPECS_FOR_SAL=1 /D STD_CALL /D DEPRECATE_DDK_FUNCTIONS=1 /D MSC_NOOPT /D _WIN32_WINNT=0x0602 /D WINVER=0x0602 /D WINNT=1 /D NTDDI_VERSION=0x06020000 /D DBG=1 /D _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1 /D KMDF_VERSION_MAJOR=1 /D KMDF_VERSION_MINOR=11 /Zp8 /Gy /Zc:wchar_t- /Zc:forScope- /GR- /wd4242 /wd4214 /wd4201 /wd4244 /wd4064 /wd4627 /wd4627 /wd4366 /wd4748 /wd4603 /wd4627 /wd4986 /wd4987 /wd4996 /wd4189 /FI"C:\Program Files (x86)\Windows Kits\8.1\Include\Shared\warning.h" /kernel /GF -cbstring /d1import_no_registry /d2AllowCompatibleILVersions /d2Zi+ driver.c
//
// link.exe  /VERSION:"6.3" /INCREMENTAL:NO /LIBPATH:"C:\Program Files (x86)\Windows Phone Kits\8.1\lib\win8\km\ARM" /WX "C:\Program Files (x86)\Windows Kits\8.1\lib\winv6.3\UM\ARM\armrt.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\BufferOverflowFastFailK.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\ntoskrnl.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\hal.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\wmilib.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\wdf\kmdf\arm\1.11\WdfLdr.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\wdf\kmdf\arm\1.11\WdfDriverEntry.lib" /NODEFAULTLIB /NODEFAULTLIB:oldnames.lib /MANIFEST:NO /DEBUG /SUBSYSTEM:NATIVE,"6.02" /STACK:"0x40000","0x2000" /Driver /OPT:REF /OPT:ICF /ENTRY:"FxDriverEntry" /RELEASE  /MERGE:"_TEXT=.text;_PAGE=PAGE" /MACHINE:ARM /PROFILE /kernel /IGNORE:4078,4221,4198 /osversion:6.3 /pdbcompress /debugtype:pdata driver.obj
//
// del ahcache.sys
// ren driver.sys ahcache.sys
//
// "C:\Program Files (x86)\Windows Kits\8.1\bin\x86\signtool.exe" sign /ph /fd "sha256" /sha1 "38DD26500D3D48F0E8C6F73F58C5F08BE77F4B7D" ahcache.sys
//
// Bluetooth profile driver
// IoAttachDeviceToDeviceStack
// WdfFdoInitSetFilter
// You don't need to query for any interfaces to use
// IOCTL_INTERNAL_BTH_SUBMIT_BRB. What l2cap functions do you want to
// test? Any particular protocols or just l2cap? You cannot arbitrarily
// send l2cap connect requests to bthport, you have to do it through an
// enumerated PDO which would be enumerated for a specific remote device's
// protocol.
// https://github.com/Microsoft/Windows-driver-samples/tree/main/setup/devcon
// HKLM\System\CurrentControlSet\Enum\BTHENUM\Dev_E0E751333260\6&23f92770&0&BluetoothDevice_E0E751333260
													// {
															// "name": "UpperFilters",
															// "value_type": "REG_MULTI_SZ",
															// "value": [
																// "mshidkmdf"
															// ]
														// }
														// {
															// "name": "Service",
															// "value_type": "REG_SZ",
															// "value": "RFCOMM"
														// },														
// https://astralvx.com/pnp-manager-device-enumeration/
// http://sviluppomobile.blogspot.com/2012/11/bluetooth-services-uuids.html		
// https://learn.microsoft.com/en-us/windows-hardware/drivers/bluetooth/installing-a-bluetooth-device
// https://learn.microsoft.com/en-us/windows-hardware/drivers/bluetooth/using-the-bluetooth-driver-stack	
// https://www.osr.com/nt-insider/2020-issue1/a-generic-device-class-filter-using-wdf/
// {2BD67D8B-8BEB-48D5-87E0-6CDA3428040A} = Device properties
// {3b2ce006-5e61-4fde-bab8-9b8aac9b26df} = System.Devices.Aep.AepId (Identity of the Device Association Endpoint)		
// {540b947e-8b40-45bc-a8a2-6a0b894cbda2} = System.Devices.Present
// {78c34fc8-104a-4aca-9ea4-524d52996e57} = System.Devices.DeviceDescription2
// {80497100-8c73-48b9-aad9-ce387e19c56e} = PKEY_Device_Reported
// {83da6326-97a6-4088-9453-a1923f573b29} = System.Devices.IsSoftwareInstalling		
// {a35996ab-11cf-4935-8b61-a6761081ecdf} = System.Devices.Aep.Category
// {a8b865dd-2e3d-4094-ad97-e593a70c75d6} =	PKEY_Device_GenericDriverInstalled	
// HardwareID = BTHENUM\\Dev_E0E751333260
// ClassGuid = {e0cbf06c-cd8b-4647-bb8a-263b43f0f974} Class = Bluetooth (This class includes all Bluetooth devices.)
// CompatibleIDs = BTHENUM\\{00001124-0000-1000-8000-00805f9b34fb}
// 						DeviceInstance = BTHENUM\\Dev_E0E751333260\\6&23f92770&0&BluetoothDevice_E0E751333260
// ServiceDiscovery 	DeviceInstance = BTHENUM\\{00001000-0000-1000-8000-00805f9b34fb}_LOCALMFG&001d\\6&23f92770&0&E0E751333260_C00000000
// HumanInterfaceDevice DeviceInstance = BTHENUM\\{00001124-0000-1000-8000-00805f9b34fb}_LOCALMFG&001d\\6&23f92770&0&E0E751333260_C00000000
// PnPInformation 		DeviceInstance = BTHENUM\\{00001200-0000-1000-8000-00805f9b34fb}_LOCALMFG&001d\\6&23f92770&0&E0E751333260_C00000000
//
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\BTHPORT\Parameters\Devices\e0e751333260
																// {
																	// "name": "COD",
																	// "value_type": "REG_DWORD",
																	// "value": "0x00002504"
																// },
// https://www.ampedrftech.com/cod.htm																
// COD 0x00002504 = Service Class "Limited Discoverable Mode" Major Device Class "Peripheral" Minor Device Class "Not Keyboard / Not Pointing Device" "Joystick"
//
// driver start = 3 (SERVICE_DEMAND_START)

// https://resources.oreilly.com/examples/9780735618039/-/tree/master/9780735618039_files/Samples			

#include "driver.h"

// prototypes
void DriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS AddDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT pdo);
NTSTATUS DispatchPnp(PDEVICE_OBJECT fdo, PIRP Irp);
NTSTATUS DispatchPower(PDEVICE_OBJECT fido, PIRP Irp);
NTSTATUS DispatchWmi(PDEVICE_OBJECT fdo, PIRP Irp);

// Logger
HANDLE openLogFile()
{
	HANDLE hLogFile;
	
	UNICODE_STRING str;
	WCHAR filepath[100]= L"\\??\\\\C:\\Data\\USERS\\Public\\Documents\\wp81wiimote.log";
	RtlInitUnicodeString(&str, filepath);
	OBJECT_ATTRIBUTES obj;	
	InitializeObjectAttributes(&obj, &str, OBJ_CASE_INSENSITIVE, NULL, NULL);

	IO_STATUS_BLOCK isb;
	NTSTATUS status = ZwCreateFile(&hLogFile, FILE_GENERIC_WRITE, &obj, &isb, 0, FILE_ATTRIBUTE_NORMAL,FILE_SHARE_WRITE, FILE_OPEN_IF,	FILE_RANDOM_ACCESS|FILE_NON_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	
	return hLogFile;
}

void debug(char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[1000];
	RtlStringCchVPrintfA(buffer, sizeof(buffer), format, args);
	
	HANDLE hLogFile = openLogFile();
	
	LARGE_INTEGER ByteOffset;

	ByteOffset.HighPart = -1;
	ByteOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
	
	size_t size;
	RtlStringCbLengthA(buffer, sizeof(buffer), &size);
	IO_STATUS_BLOCK isb;
	ZwWriteFile(hLogFile, NULL, NULL, NULL, &isb, buffer, size, &ByteOffset, NULL);
	
	ZwClose(hLogFile);
	
	va_end(args);
}

// DriverEntry
NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING  RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	
	debug("Begin DriverEntry\n");

	// Driver

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = AddDevice;

	DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
	DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
	DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = DispatchWmi; 


	debug("End DriverEntry\n");
	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT DriverObject) 
{
	UNREFERENCED_PARAMETER(DriverObject);
	
	debug("Begin DriverUnload\n");
	
	debug("End DriverUnload\n");
}

NTSTATUS AddDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT pdo)
{
	PAGED_CODE();
	
	debug("Begin AddDevice\n");
	
	NTSTATUS status;

	// Create a function device object to represent the hardware we're managing.

	PDEVICE_OBJECT fdo;
	ULONG dxsize = (sizeof(DEVICE_EXTENSION) + 7) & ~7;

	
	status = IoCreateDevice(DriverObject, dxsize, NULL, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &fdo);
	if (!NT_SUCCESS(status))
	{						// can't create device object
		debug("IoCreateDevice failed - %X\n", status);
		return status;
	}						// can't create device object

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	
	// From this point forward, any error will have side effects that need to
	// be cleaned up.
	do
	{						// finish initialization
		pdx->DeviceObject = fdo;
		pdx->Pdo = pdo;
	
		// Link our device object into the stack leading to the PDO		
		pdx->LowerDeviceObject = IoAttachDeviceToDeviceStack(fdo, pdo);
		if (!pdx->LowerDeviceObject)
		{					// can't attach
			debug("IoAttachDeviceToDeviceStack failed\n");
			status = STATUS_DEVICE_REMOVED;
			break;
		}					// can't attach

		// Set power management flags in the device object
		fdo->Flags |= DO_POWER_PAGABLE;

		// Clear the "initializing" flag so that we can get IRPs
		fdo->Flags &= ~DO_DEVICE_INITIALIZING;
	}						// finish initialization
	while (FALSE);

	////////////////////

	// PIRP Irp;
	// Irp = IoAllocateIrp( pdx->LowerDeviceObject->StackSize, FALSE );
	// Set up the next IRP stack location
	// PIO_STACK_LOCATION NextIrpStack;
	// NextIrpStack = IoGetNextIrpStackLocation( Irp );
	// NextIrpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
	// NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_BTHENUM_GET_ENUMINFO;
	// Pass the IRP down the driver stack
	// NTSTATUS Status2;
	// Status2 = IoCallDriver( pdx->LowerDeviceObject, Irp );
	// if (!NT_SUCCESS(Status2))
	// {						
		// debug("IoCallDriver failed - %X\n", status);
	// }
	// else
	// {
		// PBTH_ENUMERATOR_INFO pinfo = Irp->AssociatedIrp.SystemBuffer;
		// debug("DeviceString=%S\n", pinfo->DeviceString);
	// }
	

	////////////////////
	
	if (!NT_SUCCESS(status))
	{					// need to cleanup
		if (pdx->LowerDeviceObject)
			IoDetachDevice(pdx->LowerDeviceObject);

		IoDeleteDevice(fdo);
	}					// need to cleanup

	debug("End AddDevice\n");
	return status;
} 

NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{							
	debug("Begin DefaultPnpHandler\n");
	IoSkipCurrentIrpStackLocation(Irp);
	NTSTATUS status = IoCallDriver(pdx->LowerDeviceObject, Irp);
	debug("End DefaultPnpHandler\n");
	return status;
}			

NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}
		

NTSTATUS HandleStartDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	debug("Begin HandleStartDevice\n");
	
	Irp->IoStatus.Status = STATUS_SUCCESS;	// flag that we handled this IRP

	//NTSTATUS status = ForwardAndWait(pdx, Irp);
	// if (!NT_SUCCESS(status))
		// return CompleteRequest(Irp, status);

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	PCM_PARTIAL_RESOURCE_LIST raw;
	if (stack->Parameters.StartDevice.AllocatedResources)
		raw = &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;
	else
		raw = NULL;

	PCM_PARTIAL_RESOURCE_LIST translated;
	if (stack->Parameters.StartDevice.AllocatedResourcesTranslated)
		translated = &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;
	else
		translated = NULL;


	// Assume we are initially in the D0 state

	pdx->devpower = PowerDeviceD0;
	pdx->syspower = PowerSystemWorking;

	POWER_STATE foo;
	foo.DeviceState = PowerDeviceD0;
	PoSetPowerState(pdx->Pdo, DevicePowerState, foo);

	debug("End HandleStartDevice\n");
	return CompleteRequest(Irp, STATUS_SUCCESS);
}
		

NTSTATUS DispatchPnp(PDEVICE_OBJECT fdo, PIRP Irp)
{
	PAGED_CODE();
	
	debug("Begin DispatchPnp\n");
	
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG fcn = stack->MinorFunction;
	
	static NTSTATUS (*fcntab[])(PDEVICE_EXTENSION pdx, PIRP Irp) = {
		HandleStartDevice,		// IRP_MN_START_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_RELATIONS
		DefaultPnpHandler,		// IRP_MN_QUERY_INTERFACE
		DefaultPnpHandler,		// IRP_MN_QUERY_CAPABILITIES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_TEXT
		DefaultPnpHandler,		// IRP_MN_FILTER_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// 
		DefaultPnpHandler,		// IRP_MN_READ_CONFIG
		DefaultPnpHandler,		// IRP_MN_WRITE_CONFIG
		DefaultPnpHandler,		// IRP_MN_EJECT
		DefaultPnpHandler,		// IRP_MN_SET_LOCK
		DefaultPnpHandler,		// IRP_MN_QUERY_ID
		DefaultPnpHandler,		// IRP_MN_QUERY_PNP_DEVICE_STATE
		DefaultPnpHandler,		// IRP_MN_QUERY_BUS_INFORMATION
		DefaultPnpHandler,		// IRP_MN_DEVICE_USAGE_NOTIFICATION
		DefaultPnpHandler,		// IRP_MN_SURPRISE_REMOVAL
	};
	
	static char* fcnname[] = {
		"IRP_MN_START_DEVICE",
		"IRP_MN_QUERY_REMOVE_DEVICE",
		"IRP_MN_REMOVE_DEVICE",
		"IRP_MN_CANCEL_REMOVE_DEVICE",
		"IRP_MN_STOP_DEVICE",
		"IRP_MN_QUERY_STOP_DEVICE",
		"IRP_MN_CANCEL_STOP_DEVICE",
		"IRP_MN_QUERY_DEVICE_RELATIONS",
		"IRP_MN_QUERY_INTERFACE",
		"IRP_MN_QUERY_CAPABILITIES",
		"IRP_MN_QUERY_RESOURCES",
		"IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
		"IRP_MN_QUERY_DEVICE_TEXT",
		"IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
		"",
		"IRP_MN_READ_CONFIG",
		"IRP_MN_WRITE_CONFIG",
		"IRP_MN_EJECT",
		"IRP_MN_SET_LOCK",
		"IRP_MN_QUERY_ID",
		"IRP_MN_QUERY_PNP_DEVICE_STATE",
		"IRP_MN_QUERY_BUS_INFORMATION",
		"IRP_MN_DEVICE_USAGE_NOTIFICATION",
		"IRP_MN_SURPRISE_REMOVAL",
	};
	if (fcn < arraysize(fcnname))
		debug("PNP Request (%s)\n", fcnname[fcn]);
	else
		debug("PNP Request ?? (0x%X)\n", fcn);
	
	if (fcn >= arraysize(fcntab))
	{						// unknown function
		NTSTATUS status = DefaultPnpHandler(pdx, Irp); // some function we don't know about
		debug("End DispatchPnp\n");
		return status;
	}						// unknown function	
	
	NTSTATUS status = (*fcntab[fcn])(pdx, Irp);
	
	debug("End DispatchPnp\n");
	return status;
} 

NTSTATUS DispatchPower(PDEVICE_OBJECT fdo, PIRP Irp)
{
	UNREFERENCED_PARAMETER(fdo);
	
	debug("Begin DispatchPower\n");
	
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG fcn = stack->MinorFunction;
	
	static char* fcnname[] = {
		"IRP_MN_WAIT_WAKE",
		"IRP_MN_POWER_SEQUENCE",
		"IRP_MN_SET_POWER",
		"IRP_MN_QUERY_POWER",
	};
	
	static char* sysstate[] = {
		"PowerSystemUnspecified",
		"PowerSystemWorking",
		"PowerSystemSleeping1",
		"PowerSystemSleeping2",
		"PowerSystemSleeping3",
		"PowerSystemHibernate",
		"PowerSystemShutdown",
		"PowerSystemMaximum",
	};
	
	static char* devstate[] = {
		"PowerDeviceUnspecified",
		"PowerDeviceD0",
		"PowerDeviceD1",
		"PowerDeviceD2",
		"PowerDeviceD3",
		"PowerDeviceMaximum",
	};

	if (fcn == IRP_MN_SET_POWER || fcn == IRP_MN_QUERY_POWER)
	{						// handle set/query

		if (stack->Parameters.Power.Type == SystemPowerState)
			debug("POWER Request (%s), S-state = %s\n", fcnname[fcn], sysstate[stack->Parameters.Power.State.SystemState]);
		else
			debug("POWER Request (%s), D-state = %s\n", fcnname[fcn], devstate[stack->Parameters.Power.State.DeviceState]);

	}
	else
	{
		if (fcn < arraysize(fcnname))
			debug("POWER Request (%s)\n", fcnname[fcn]);
		else
			debug("POWER Request ?? (0x%X)\n", fcn);
		
	}

	
	debug("End DispatchPower\n");
	return STATUS_SUCCESS;
}

NTSTATUS DispatchWmi(PDEVICE_OBJECT fdo, PIRP Irp)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(Irp);
	
	debug("Begin DispatchWmi\n");
	
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	IoSkipCurrentIrpStackLocation(Irp);

	debug("End DispatchWmi\n");
	return IoCallDriver(pdx->LowerDeviceObject, Irp);
}

