// bcdedit /store f:\EFIESP\efi\Microsoft\Boot\BCD /set {default} bootlog Yes
// bcdedit /store f:\EFIESP\efi\Microsoft\Boot\BCD /set {default} testsigning yes
//
// set PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\x86_arm;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin;%PATH%
//
// CL.exe /c /I"C:\Program Files (x86)\Windows Phone Kits\8.1\Include\km" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\Shared" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\km" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\wdf\kmdf\1.11" /I"C:\Program Files (x86)\Windows Kits\8.1\Include\km\crt" /Zi /W4 /WX /Od /D _ARM_ /D ARM /D _USE_DECLSPECS_FOR_SAL=1 /D STD_CALL /D DEPRECATE_DDK_FUNCTIONS=1 /D MSC_NOOPT /D _WIN32_WINNT=0x0602 /D WINVER=0x0602 /D WINNT=1 /D NTDDI_VERSION=0x06020000 /D DBG=1 /D _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE=1 /D KMDF_VERSION_MAJOR=1 /D KMDF_VERSION_MINOR=11 /Zp8 /Gy /Zc:wchar_t- /Zc:forScope- /GR- /wd4242 /wd4214 /wd4201 /wd4244 /wd4064 /wd4627 /wd4627 /wd4366 /wd4748 /wd4603 /wd4627 /wd4986 /wd4987 /wd4996 /wd4189 /FI"C:\Program Files (x86)\Windows Kits\8.1\Include\Shared\warning.h" /kernel /GF -cbstring /d1import_no_registry /d2AllowCompatibleILVersions /d2Zi+ driver.c
//
// link.exe  /VERSION:"6.3" /INCREMENTAL:NO /LIBPATH:"C:\Program Files (x86)\Windows Phone Kits\8.1\lib\win8\km\ARM" /WX "C:\Program Files (x86)\Windows Kits\8.1\lib\winv6.3\UM\ARM\armrt.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\BufferOverflowFastFailK.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\ntoskrnl.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\hal.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\win8\KM\arm\wmilib.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\wdf\kmdf\arm\1.11\WdfLdr.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\wdf\kmdf\arm\1.11\WdfDriverEntry.lib" /NODEFAULTLIB /NODEFAULTLIB:oldnames.lib /MANIFEST:NO /DEBUG /SUBSYSTEM:NATIVE,"6.02" /STACK:"0x40000","0x2000" /Driver /OPT:REF /OPT:ICF /ENTRY:"FxDriverEntry" /RELEASE  /MERGE:"_TEXT=.text;_PAGE=PAGE" /MACHINE:ARM /PROFILE /kernel /IGNORE:4078,4221,4198 /osversion:6.3 /pdbcompress /debugtype:pdata driver.obj
//
//
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
//
// https://resources.oreilly.com/examples/9780735618039/-/tree/master/9780735618039_files/Samples			
// 
// Local  address: 78:92:3E:C7:7B:53 (132569513818963)
// Black Device address: E0:E7:51:33:32:60 (247284104376928)
// White Device address: 00:19:FD:D6:1C:84 (111632850052)
//
// BTHENUM#{0000110c-0000-1000-8000-00805f9b34fb}_LOCALMFG&0000#6&23f92770&0&000000000000_00000000#{0000110c-0000-1000-8000-00805f9b34fb}
//
// Bluetooth class {e0cbf06c-cd8b-4647-bb8a-263b43f0f974}
//
// wp81wiimote (upper filter)
//   |
// BthEnum
//   |
// BthPort
//   |
// BthUSB
//
//
// bthl2cap (upper filter)
//   |
// bthci
//
// HKEY_CURRENT_USER\SYSTEM\ControlSet001\Enum\SystemBusQc\SMD_BT\4&315a27b&0&4097\LocationInformation=Serial HCI Bus - Bluetooth Function
// HKEY_CURRENT_USER\SYSTEM\ControlSet001\Enum\SystemBusQc\SMD_BT\4&315a27b&0&4097\Device Parameters\Disconnect Timeout=30000
// HKEY_CURRENT_USER\SYSTEM\ControlSet001\Enum\SystemBusQc\SMD_BT\4&315a27b&0&4097\Device Parameters\SymbolicLinkName=\??\SystemBusQc#SMD_BT#4&315a27b&0&4097#{0850302a-b344-4fda-9be9-90576b8d46f0}\0003
// HKEY_CURRENT_USER\SYSTEM\ControlSet001\Services\BtConnMgr\BthPortDeviceParameters\Disconnect Timeout=30000
//
// https://www.lisha.ufsc.br/teaching/shi/ine5346-2003-1/work/bluetooth/hci_commands.html
// 
// {"GUID" : "8a1f9517-3a8c-4a9e-a018-4f17a200f277", "Name" : "Microsoft-Windows-BTH-BTHPORT"}
// 17 95 1f 8a 8c 3a 9e 4a a0 18 4f 17 a2 00 f2 77
//
// {"GUID" : "ee6150ef-c97e-46dd-b635-4c047599fc0f", "Name" : "Microsoft-WindowsPhone-Bluetooth-CMCL"}
// {"GUID" : "A9B5617D-6C0E-4adc-B076-F3B0CDD4D0ED", "Name" : "Microsoft-WindowsPhone-Bluetooth-CM"}
// 7d 61 b5 a9 0e 6c-dc 4a b0 76 f3 b0 cd d4 d0 ed
//
// SYSTEM\\ControlSet001\\Services\\BTConnMgr\\Parameters
//		DebugConsoleEnabled 0/1
//		DebugSevLevel 0/1/2/3
//
// Device_UpdateWaitPaired
// 10023a9e 20 46           mov param_1,r4
// 10023aa0 ff f7 fa f9     bl Device_SetTimer
//
// HKEY_CURRENT_USER\SYSTEM\ControlSet001\Enum\SystemBusQc\SMD_BT\4&315a27b&0&4097
// HKEY_CURRENT_USER\SYSTEM\ControlSet001\Enum\SystemBusQc\SMD_BT\4&315a27b&0&4097\Device Parameters, SymbolicLinkName, \??\SystemBusQc#SMD_BT#4&315a27b&0&4097#{0850302a-b344-4fda-9be9-90576b8d46f0}
//
// https://stackoverflow.com/questions/2643084/sysinternals-winobj-device-listing-mechanism


#include <ntifs.h>
#include <wdf.h>
#include "device.h"

void EvtCleanupCallback(WDFOBJECT DriverObject) 
{
    UNREFERENCED_PARAMETER(DriverObject);
	
	DbgPrint("WII!EvtCleanupCallback");
}

// DriverEntry
NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING  RegistryPath)
{
	DbgPrint("WII!Begin DriverEntry");
	
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

	DbgPrint("WII!Driver registryPath= %S", RegistryPath->Buffer);
	DbgPrint("WII!DriverName= %S", DriverObject->DriverName.Buffer);
	DbgPrint("WII!HardwareDatabase= %S", DriverObject->HardwareDatabase->Buffer);

	DbgPrint("WII!End DriverEntry");
    return status;
}


