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
// driver start = 3 (SERVICE_DEMAND_START)

			

#include "driver.h"
#include <initguid.h>
#include "hidclass.h"
#include "Wdmguid.h"

// prototypes

void WiimoteUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS WiimoteCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS WiimoteDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
DRIVER_NOTIFICATION_CALLBACK_ROUTINE MyCallbackRoutine;

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
// chapter 06, https://www.programcreek.com/cpp/?project_name=zodiacon%2Fwindowskernelprogrammingbook#

NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING  RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	
	NTSTATUS status;
	
	debug("Begin DriverEntry\n");

	// Driver

	DriverObject->DriverUnload = WiimoteUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = WiimoteCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = WiimoteCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WiimoteDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\WP81Wiimote");
	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		debug("Failed to create device (0x%08X)\n", status);
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WP81Wiimote");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		debug("Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}

	debug("End DriverEntry\n");
	return STATUS_SUCCESS;
}

void WiimoteUnload(_In_ PDRIVER_OBJECT DriverObject) {
	debug("Begin WiimoteUnload\n");
	
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WP81Wiimote");
	// delete symbolic link
	IoDeleteSymbolicLink(&symLink);

	// delete device object
	IoDeleteDevice(DriverObject->DeviceObject);
	
	debug("End WiimoteUnload\n");
}

_Use_decl_annotations_
NTSTATUS WiimoteCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	debug("Begin WiimoteCreateClose\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
	debug("End WiimoteCreateClose\n");
	
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS WiimoteDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	//UNREFERENCED_PARAMETER(DeviceObject);
	
	debug("Begin WiimoteDeviceControl\n");
	
	// get our IO_STACK_LOCATION
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_WIIMOTE_TEST:
		{
			// do the work
			debug("Received IOCTL_WIIMOTE_TEST\n");
			
			HANDLE *pInputBuffer = (HANDLE*)stack->Parameters.DeviceIoControl.Type3InputBuffer; // METHOD_NEITHER
			HANDLE radioHandle = *pInputBuffer;
			debug("radioHandle=0x%08X\n",radioHandle);
			
			PVOID NotificationHandle = NULL;
			NotificationHandle = ExAllocatePool(PagedPool,1024);
			
			NTSTATUS status2 = IoRegisterPlugPlayNotification (
                EventCategoryDeviceInterfaceChange,
                PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                (PVOID)&GUID_DEVINTERFACE_HID,
                DeviceObject->DriverObject,
                MyCallbackRoutine,
                DeviceObject,
                NotificationHandle);

			if (!NT_SUCCESS(status2)) {
				debug("status2=%d\n", status2);
			}
			debug("NotificationHandle=0x%08X\n", NotificationHandle);
			ExFreePool(NotificationHandle);
			
			// PVOID radioObject;
			// NTSTATUS status2 = ObReferenceObjectByHandle(radioHandle, GENERIC_ALL, NULL, UserMode, &radioObject, NULL);
			// debug("status2=%d\n",status2);
			// if (NT_SUCCESS(status2)) 
			// {
				// debug("radioObject=0x%08X\n",radioObject);
				
				// KEVENT hComplete;
				// KeInitializeEvent(&hComplete,NotificationEvent,FALSE);

				// BTH_LOCAL_RADIO_INFO radioInfo1;
				// BTH_LOCAL_RADIO_INFO radioInfo2;

				// IO_STATUS_BLOCK IoStatusBlock;				
				// PIRP pIrp = IoBuildDeviceIoControlRequest(
					// IOCTL_BTH_GET_LOCAL_INFO,
					// radioObject,
					// &radioInfo1,sizeof(radioInfo1),
					// &radioInfo2,sizeof(radioInfo2),FALSE,&hComplete,&IoStatusBlock);
				
				// NTSTATUS status3 = IoCallDriver(radioObject,pIrp);
				// debug(hLogFile, "status3=%d\n",status3);
				// if (status3==STATUS_PENDING)
					// KeWaitForSingleObject(&hComplete,Suspended,KernelMode,FALSE,NULL);
				// else
					// IoStatusBlock.Status=status3;
				
				// debug("flags1=0x%08X\n",radioInfo1.flags);
				// debug("flags2=0x%08X\n",radioInfo2.flags);
								
				// ObDereferenceObject(radioObject);
			// }
			break;
		}

		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
	debug("End WiimoteDeviceControl\n");
	
	return status;
}

NTSTATUS MyCallbackRoutine(PVOID NotificationStructure, PVOID Context)
{
	//UNREFERENCED_PARAMETER(NotificationStructure);
	UNREFERENCED_PARAMETER(Context);
	
	NTSTATUS status = STATUS_SUCCESS;
	
	debug("Begin MyCallbackRoutine\n");
	
	DEVICE_INTERFACE_CHANGE_NOTIFICATION* pNotification = NULL;
    pNotification = (DEVICE_INTERFACE_CHANGE_NOTIFICATION*)NotificationStructure;
	
	debug("pNotification->Version = %d\n",pNotification->Version);	
	
	GUID eventGuid = pNotification->Event;	
	GUID interfaceClassGuid = pNotification->InterfaceClassGuid;	
	
	UNICODE_STRING evenGuidString;
	RtlStringFromGUID(&eventGuid, &evenGuidString);
	debug("pNotification->Event = %wZ",&evenGuidString);
	RtlFreeUnicodeString(&evenGuidString);
	if (RtlEqualMemory(&eventGuid, &GUID_DEVICE_INTERFACE_ARRIVAL, sizeof(GUID)))
    {
		debug("\tGUID_DEVICE_INTERFACE_ARRIVAL\n");
	}
	if (RtlEqualMemory(&eventGuid, &GUID_DEVICE_INTERFACE_REMOVAL, sizeof(GUID)))
    {
		debug("\tGUID_DEVICE_INTERFACE_REMOVAL\n");
	}

	UNICODE_STRING interfaceClassGuidString;
	RtlStringFromGUID(&interfaceClassGuid, &interfaceClassGuidString);
	debug("pNotification->InterfaceClassGuid = %wZ\n",&interfaceClassGuidString);
	RtlFreeUnicodeString(&interfaceClassGuidString);

	debug("pNotification->SymbolicLinkName = %wZ\n", pNotification->SymbolicLinkName);
	
	debug("End MyCallbackRoutine\n");
	
	return status;
}