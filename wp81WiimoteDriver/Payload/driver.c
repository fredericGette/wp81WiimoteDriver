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

#include "driver.h"

HANDLE hLogFile;

// prototypes

void WiimoteUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS WiimoteCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS WiimoteDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

// Logger

void write2File(HANDLE hFile, char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[1000];
	RtlStringCchVPrintfA(buffer, sizeof(buffer), format, args);
	
	size_t size;
	RtlStringCbLengthA(buffer, sizeof(buffer), &size);
	IO_STATUS_BLOCK isb;
	ZwWriteFile(hFile, NULL, NULL, NULL, &isb, buffer, size, NULL, NULL);
	
	va_end(args);
}

// DriverEntry
// chapter 06, https://www.programcreek.com/cpp/?project_name=zodiacon%2Fwindowskernelprogrammingbook#

NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING  RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	
	NTSTATUS status;
	
	// Logger
	UNICODE_STRING str;
	WCHAR filepath[100]= L"\\??\\\\C:\\Data\\USERS\\Public\\Documents\\driver.log";
	RtlInitUnicodeString(&str, filepath);
	OBJECT_ATTRIBUTES obj;	
	InitializeObjectAttributes(&obj, &str, OBJ_CASE_INSENSITIVE, NULL, NULL);

	IO_STATUS_BLOCK isb;
	status = ZwCreateFile(&hLogFile, FILE_GENERIC_WRITE, &obj, &isb, 0, FILE_ATTRIBUTE_NORMAL,FILE_SHARE_WRITE, FILE_OVERWRITE_IF,	FILE_RANDOM_ACCESS|FILE_NON_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	
	write2File(hLogFile, "DriverEntry\n");

	// Driver

	DriverObject->DriverUnload = WiimoteUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = WiimoteCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = WiimoteCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WiimoteDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\Wiimote");
	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		write2File(hLogFile,"Failed to create device (0x%08X)\n", status);
		ZwClose(hLogFile);
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Wiimote");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		write2File(hLogFile,"Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
		ZwClose(hLogFile);
		return status;
	}

	ZwClose(hLogFile);

	return STATUS_SUCCESS;
}

void WiimoteUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Wiimote");
	// delete symbolic link
	IoDeleteSymbolicLink(&symLink);

	// delete device object
	IoDeleteDevice(DriverObject->DeviceObject);
}

_Use_decl_annotations_
NTSTATUS WiimoteCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS WiimoteDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	
	// get our IO_STACK_LOCATION
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_WIIMOTE_TEST:
		{
			// do the work
			break;
		}

		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// int main(int argc, const char* argv[]) {
	// if (argc < 3) {
		// printf("Usage: Booster <threadid> <priority>\n");
		// return 0;
	// }

	// HANDLE hDevice = CreateFile(L"\\\\.\\PriorityBooster", GENERIC_WRITE, FILE_SHARE_WRITE,
		// nullptr, OPEN_EXISTING, 0, nullptr);
	// if (hDevice == INVALID_HANDLE_VALUE)
		// return Error("Failed to open device");

	// ThreadData data;
	// data.ThreadId = atoi(argv[1]);
	// data.Priority = atoi(argv[2]);

	// DWORD returned;
	// BOOL success = DeviceIoControl(hDevice, IOCTL_PRIORITY_BOOSTER_SET_PRIORITY, &data, sizeof(data), nullptr, 0, &returned, nullptr);
	// if (success)
		// printf("Priority change succeeded!\n");
	// else
		// Error("Priority change failed!");

	// CloseHandle(hDevice);
// }