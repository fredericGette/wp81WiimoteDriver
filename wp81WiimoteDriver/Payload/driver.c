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
//
// https://resources.oreilly.com/examples/9780735618039/-/tree/master/9780735618039_files/Samples			
// 
// Local  address: 78:92:3E:C7:7B:53 (132569513818963)
// Device address: E0:E7:51:33:32:60 (247284104376928)

#include <ntifs.h>
#include <wdf.h>
#include "log.h"
#include "device.h"

void EvtCleanupCallback(WDFOBJECT DriverObject) 
{
    UNREFERENCED_PARAMETER(DriverObject);
	
	debug("Begin EvtCleanupCallback\n");
	debug("End EvtCleanupCallback\n");
}

// DriverEntry
NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING  RegistryPath)
{
	debug("Begin DriverEntry\n");
	
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

	debug("End DriverEntry\n");
    return status;
}


