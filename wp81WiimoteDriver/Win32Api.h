#pragma once

// See https://github.com/tandasat/SecRuntimeSample/blob/master/SecRuntimeSampleNative/Win32Api.h

//
// Service Control Manager object specific access types
//
#define SC_MANAGER_CONNECT             0x0001
#define SC_MANAGER_CREATE_SERVICE      0x0002
#define SC_MANAGER_ENUMERATE_SERVICE   0x0004
#define SC_MANAGER_LOCK                0x0008
#define SC_MANAGER_QUERY_LOCK_STATUS   0x0010
#define SC_MANAGER_MODIFY_BOOT_CONFIG  0x0020

#define SC_MANAGER_ALL_ACCESS          (STANDARD_RIGHTS_REQUIRED      | \
                                        SC_MANAGER_CONNECT            | \
                                        SC_MANAGER_CREATE_SERVICE     | \
                                        SC_MANAGER_ENUMERATE_SERVICE  | \
                                        SC_MANAGER_LOCK               | \
                                        SC_MANAGER_QUERY_LOCK_STATUS  | \
                                        SC_MANAGER_MODIFY_BOOT_CONFIG)

//
// Service object specific access type
//
#define SERVICE_QUERY_CONFIG           0x0001
#define SERVICE_CHANGE_CONFIG          0x0002
#define SERVICE_QUERY_STATUS           0x0004
#define SERVICE_ENUMERATE_DEPENDENTS   0x0008
#define SERVICE_START                  0x0010
#define SERVICE_STOP                   0x0020
#define SERVICE_PAUSE_CONTINUE         0x0040
#define SERVICE_INTERROGATE            0x0080
#define SERVICE_USER_DEFINED_CONTROL   0x0100

#define SERVICE_ALL_ACCESS             (STANDARD_RIGHTS_REQUIRED     | \
                                        SERVICE_QUERY_CONFIG         | \
                                        SERVICE_CHANGE_CONFIG        | \
                                        SERVICE_QUERY_STATUS         | \
                                        SERVICE_ENUMERATE_DEPENDENTS | \
                                        SERVICE_START                | \
                                        SERVICE_STOP                 | \
                                        SERVICE_PAUSE_CONTINUE       | \
                                        SERVICE_INTERROGATE          | \
                                        SERVICE_USER_DEFINED_CONTROL)

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

typedef ACCESS_MASK REGSAM;

typedef struct _STARTUPINFOA {
	DWORD cb;
	LPSTR lpReserved;
	LPSTR lpDesktop;
	LPSTR lpTitle;
	DWORD dwX;
	DWORD dwY;
	DWORD dwXSize;
	DWORD dwYSize;
	DWORD dwXCountChars;
	DWORD dwYCountChars;
	DWORD dwFillAttribute;
	DWORD dwFlags;
	WORD wShowWindow;
	WORD cbReserved2;
	LPBYTE lpReserved2;
	HANDLE hStdInput;
	HANDLE hStdOutput;
	HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;

typedef struct _PROCESS_INFORMATION {
	HANDLE hProcess;
	HANDLE hThread;
	DWORD dwProcessId;
	DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

DECLARE_HANDLE(SC_HANDLE);
typedef SC_HANDLE   *LPSC_HANDLE;

typedef struct _BLUETOOTH_FIND_RADIO_PARAMS {
	DWORD   dwSize;             //  IN  sizeof this structure
} BLUETOOTH_FIND_RADIO_PARAMS;

typedef HANDLE      HBLUETOOTH_RADIO_FIND;

typedef _Return_type_success_(return == 0) DWORD        RETURN_TYPE;
typedef RETURN_TYPE  CONFIGRET;

typedef _Null_terminated_ WCHAR *DEVNODEID_W, *DEVINSTID_W;

//
// Flags for CM_Get_Device_Interface_List, CM_Get_Device_Interface_List_Size
//
#define CM_GET_DEVICE_INTERFACE_LIST_PRESENT     (0x00000000)  // only currently 'live' device interfaces
#define CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES (0x00000001)  // all registered device interfaces, live or not
#define CM_GET_DEVICE_INTERFACE_LIST_BITS        (0x00000001)

#define CR_SUCCESS                  (0x00000000)

#define BLUETOOTH_MAX_NAME_SIZE             (248)

typedef ULONGLONG BTH_ADDR;

typedef struct _BLUETOOTH_ADDRESS {
	union {
		BTH_ADDR ullLong;       //  easier to compare again BLUETOOTH_NULL_ADDRESS
		BYTE    rgBytes[6];   //  easier to format when broken out
	};

} BLUETOOTH_ADDRESS_STRUCT;

#define BLUETOOTH_ADDRESS BLUETOOTH_ADDRESS_STRUCT

typedef struct _BLUETOOTH_DEVICE_INFO {
	_Field_range_(== , sizeof(BLUETOOTH_DEVICE_INFO_STRUCT))
		DWORD   dwSize;                             //  size, in bytes, of this structure - must be the sizeof(BLUETOOTH_DEVICE_INFO)

	BLUETOOTH_ADDRESS Address;                  //  Bluetooth address

	ULONG   ulClassofDevice;                    //  Bluetooth "Class of Device"

	BOOL    fConnected;                         //  Device connected/in use
	BOOL    fRemembered;                        //  Device remembered
	BOOL    fAuthenticated;                     //  Device authenticated/paired/bonded

	SYSTEMTIME  stLastSeen;                     //  Last time the device was seen
	SYSTEMTIME  stLastUsed;                     //  Last time the device was used for other than RNR, inquiry, or SDP

	WCHAR   szName[BLUETOOTH_MAX_NAME_SIZE];  //  Name of the device

} BLUETOOTH_DEVICE_INFO_STRUCT;

#define BLUETOOTH_DEVICE_INFO BLUETOOTH_DEVICE_INFO_STRUCT

typedef struct _BLUETOOTH_RADIO_INFO {
	DWORD dwSize;                               // Size, in bytes, of this entire data structure

	BLUETOOTH_ADDRESS address;                  // Address of the local radio

	WCHAR szName[BLUETOOTH_MAX_NAME_SIZE];    // Name of the local radio

	ULONG ulClassofDevice;                      // Class of device for the local radio

	USHORT lmpSubversion;                       // lmpSubversion, manufacturer specifc.
	USHORT manufacturer;                        // Manufacturer of the radio, BTH_MFG_Xxx value.  For the most up to date
												// list, goto the Bluetooth specification website and get the Bluetooth
												// assigned numbers document.
} BLUETOOTH_RADIO_INFO, *PBLUETOOTH_RADIO_INFO;

typedef struct _BLUETOOTH_DEVICE_SEARCH_PARAMS {
	DWORD   dwSize;                 //  IN  sizeof this structure

	BOOL    fReturnAuthenticated;   //  IN  return authenticated devices
	BOOL    fReturnRemembered;      //  IN  return remembered devices
	BOOL    fReturnUnknown;         //  IN  return unknown devices
	BOOL    fReturnConnected;       //  IN  return connected devices

	BOOL    fIssueInquiry;          //  IN  issue a new inquiry
	UCHAR   cTimeoutMultiplier;     //  IN  timeout for the inquiry

	HANDLE  hRadio;                 //  IN  handle to radio to enumerate - NULL == all radios will be searched

} BLUETOOTH_DEVICE_SEARCH_PARAMS;

typedef HANDLE      HBLUETOOTH_DEVICE_FIND;

extern "C" {
	WINBASEAPI HMODULE WINAPI LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	WINBASEAPI HMODULE WINAPI GetModuleHandleW(LPCWSTR lpModuleName);

	LONG WINAPI RegOpenKeyExW(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);
	LONG WINAPI RegQueryValueExW(HKEY, LPCWSTR, PDWORD, PDWORD, LPBYTE, PDWORD);
	LONG WINAPI RegCloseKey(HKEY);
	LONG WINAPI RegQueryInfoKeyW(HKEY, LPWSTR, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PFILETIME);
	LONG WINAPI RegEnumKeyExW(HKEY, DWORD, LPWSTR, PDWORD, PDWORD, LPWSTR, PDWORD, PFILETIME);
	LONG WINAPI RegEnumValueW(HKEY, DWORD, LPWSTR, PDWORD, PDWORD, PDWORD, LPBYTE, PDWORD);
	LONG WINAPI RegSetValueExW(HKEY, LPCWSTR, DWORD, DWORD, const BYTE*, DWORD);
	LONG WINAPI RegCreateKeyExW(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, PDWORD);

	WINBASEAPI HANDLE WINAPI FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
	WINBASEAPI BOOL WINAPI FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
	WINBASEAPI BOOL WINAPI FindClose(HANDLE hFindFile);
	WINBASEAPI HANDLE WINAPI CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
	WINBASEAPI BOOL WINAPI WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

	WINBASEAPI BOOL WINAPI CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
	WINBASEAPI BOOL WINAPI CloseHandle(HANDLE hObject);

	WINBASEAPI BOOL WINAPI DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
	WINBASEAPI BOOL WINAPI EnumDeviceDrivers(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded);
	WINBASEAPI DWORD WINAPI GetDeviceDriverBaseNameW(LPVOID ImageBase, LPWSTR lpBaseName, DWORD nSize);

	WINBASEAPI BOOL	WINAPI CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);

	WINADVAPI SC_HANDLE WINAPI OpenSCManagerW(LPCWSTR lpMachineName, LPCWSTR lpDatabaseName, DWORD dwDesiredAccess);
	WINADVAPI SC_HANDLE WINAPI CreateServiceW(SC_HANDLE hSCManager, LPCWSTR lpServiceName, LPCWSTR lpDisplayName, DWORD dwDesiredAccess, DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl, LPCWSTR lpBinaryPathName, LPCWSTR lpLoadOrderGroup, LPDWORD lpdwTagId, LPCWSTR lpDependencies, LPCWSTR lpServiceStartName, LPCWSTR lpPassword);
	WINADVAPI BOOL WINAPI CloseServiceHandle(SC_HANDLE hSCObject);

	HBLUETOOTH_RADIO_FIND WINAPI BluetoothFindFirstRadio(const BLUETOOTH_FIND_RADIO_PARAMS * pbtfrp, HANDLE * phRadio);
	DWORD WINAPI BluetoothGetRadioInfo(HANDLE hRadio, PBLUETOOTH_RADIO_INFO pRadioInfo);
	HBLUETOOTH_DEVICE_FIND WINAPI BluetoothFindFirstDevice(const BLUETOOTH_DEVICE_SEARCH_PARAMS * pbtsp, BLUETOOTH_DEVICE_INFO * pbtdi);
	BOOL WINAPI BluetoothFindNextDevice(HBLUETOOTH_DEVICE_FIND  hFind, BLUETOOTH_DEVICE_INFO * pbtdi);

	CMAPI CONFIGRET	WINAPI CM_Get_Device_Interface_List_SizeW(PULONG pulLen,LPGUID InterfaceClassGuid, DEVINSTID_W pDeviceID, ULONG ulFlags);
	CMAPI CONFIGRET WINAPI CM_Get_Device_Interface_ListW(LPGUID InterfaceClassGuid, DEVINSTID_W pDeviceID, PZZWSTR Buffer, ULONG BufferLen, ULONG ulFlags);

}

#define WIN32API_TOSTRING(x) #x

// Link exported function
#define WIN32API_INIT_PROC(Module, Name)  \
  Name(reinterpret_cast<decltype(&::Name)>( \
      ::GetProcAddress((Module), WIN32API_TOSTRING(Name))))

// Convenientmacro to declare function
#define WIN32API_DEFINE_PROC(Name) const decltype(&::Name) Name

class Win32Api {

private:
	// Returns a base address of KernelBase.dll
	static HMODULE GetKernelBase() {
		return GetBaseAddress(&::DisableThreadLibraryCalls);
	}

	// Returns a base address of the given address
	static HMODULE GetBaseAddress(const void *Address) {
		MEMORY_BASIC_INFORMATION mbi = {};
		if (!::VirtualQuery(Address, &mbi, sizeof(mbi))) {
			return nullptr;
		}
		const auto mz = *reinterpret_cast<WORD *>(mbi.AllocationBase);
		if (mz != IMAGE_DOS_SIGNATURE) {
			return nullptr;
		}
		return reinterpret_cast<HMODULE>(mbi.AllocationBase);
	}

public:
	const HMODULE m_Kernelbase;
	WIN32API_DEFINE_PROC(LoadLibraryExW);
	WIN32API_DEFINE_PROC(GetModuleHandleW);
	WIN32API_DEFINE_PROC(RegOpenKeyExW);
	WIN32API_DEFINE_PROC(RegQueryValueExW);
	WIN32API_DEFINE_PROC(RegCloseKey);
	WIN32API_DEFINE_PROC(RegQueryInfoKeyW);
	WIN32API_DEFINE_PROC(RegEnumKeyExW);
	WIN32API_DEFINE_PROC(RegEnumValueW);
	WIN32API_DEFINE_PROC(RegSetValueExW);
	WIN32API_DEFINE_PROC(RegCreateKeyExW);
	WIN32API_DEFINE_PROC(FindFirstFileW);
	WIN32API_DEFINE_PROC(FindNextFileW);
	WIN32API_DEFINE_PROC(FindClose);
	WIN32API_DEFINE_PROC(CreateFileW);
	WIN32API_DEFINE_PROC(WriteFile);
	WIN32API_DEFINE_PROC(CreateProcessA);
	WIN32API_DEFINE_PROC(CloseHandle);
	WIN32API_DEFINE_PROC(DeviceIoControl);
	WIN32API_DEFINE_PROC(EnumDeviceDrivers);
	WIN32API_DEFINE_PROC(GetDeviceDriverBaseNameW);
	const HMODULE m_Kernel32legacy;
	WIN32API_DEFINE_PROC(CopyFileW);
	const HMODULE m_SecHost;
	WIN32API_DEFINE_PROC(OpenSCManagerW);
	WIN32API_DEFINE_PROC(CreateServiceW);
	WIN32API_DEFINE_PROC(CloseServiceHandle);
	const HMODULE m_BluetoothApis;
	WIN32API_DEFINE_PROC(BluetoothFindFirstRadio);
	WIN32API_DEFINE_PROC(BluetoothGetRadioInfo);
	WIN32API_DEFINE_PROC(BluetoothFindFirstDevice);
	WIN32API_DEFINE_PROC(BluetoothFindNextDevice);
	const HMODULE m_CfgMgr32;
	WIN32API_DEFINE_PROC(CM_Get_Device_Interface_List_SizeW);
	WIN32API_DEFINE_PROC(CM_Get_Device_Interface_ListW);

	Win32Api()
		: m_Kernelbase(GetKernelBase()),
		WIN32API_INIT_PROC(m_Kernelbase, LoadLibraryExW),
		WIN32API_INIT_PROC(m_Kernelbase, GetModuleHandleW),
		WIN32API_INIT_PROC(m_Kernelbase, RegOpenKeyExW),
		WIN32API_INIT_PROC(m_Kernelbase, RegQueryValueExW),
		WIN32API_INIT_PROC(m_Kernelbase, RegCloseKey),
		WIN32API_INIT_PROC(m_Kernelbase, RegQueryInfoKeyW),
		WIN32API_INIT_PROC(m_Kernelbase, RegEnumKeyExW),
		WIN32API_INIT_PROC(m_Kernelbase, RegEnumValueW),
		WIN32API_INIT_PROC(m_Kernelbase, RegSetValueExW),
		WIN32API_INIT_PROC(m_Kernelbase, RegCreateKeyExW),
		WIN32API_INIT_PROC(m_Kernelbase, FindFirstFileW),
		WIN32API_INIT_PROC(m_Kernelbase, FindNextFileW),
		WIN32API_INIT_PROC(m_Kernelbase, FindClose),
		WIN32API_INIT_PROC(m_Kernelbase, CreateFileW),
		WIN32API_INIT_PROC(m_Kernelbase, WriteFile),
		WIN32API_INIT_PROC(m_Kernelbase, CreateProcessA),
		WIN32API_INIT_PROC(m_Kernelbase, CloseHandle),
		WIN32API_INIT_PROC(m_Kernelbase, DeviceIoControl),
		WIN32API_INIT_PROC(m_Kernelbase, EnumDeviceDrivers),
		WIN32API_INIT_PROC(m_Kernelbase, GetDeviceDriverBaseNameW),
		m_Kernel32legacy(GetModuleHandleW(L"KERNEL32LEGACY.DLL")),
		WIN32API_INIT_PROC(m_Kernel32legacy, CopyFileW),
		m_SecHost(GetModuleHandleW(L"SECHOST.DLL")),
		WIN32API_INIT_PROC(m_SecHost, OpenSCManagerW),
		WIN32API_INIT_PROC(m_SecHost, CreateServiceW),
		WIN32API_INIT_PROC(m_SecHost, CloseServiceHandle),
		m_BluetoothApis(LoadLibraryExW(L"BLUETOOTHAPIS.DLL", NULL, NULL)),
		WIN32API_INIT_PROC(m_BluetoothApis, BluetoothFindFirstRadio),
		WIN32API_INIT_PROC(m_BluetoothApis, BluetoothGetRadioInfo),
		WIN32API_INIT_PROC(m_BluetoothApis, BluetoothFindFirstDevice),
		WIN32API_INIT_PROC(m_BluetoothApis, BluetoothFindNextDevice),
		m_CfgMgr32(LoadLibraryExW(L"CFGMGR32.DLL", NULL, NULL)),
		WIN32API_INIT_PROC(m_CfgMgr32, CM_Get_Device_Interface_List_SizeW),
		WIN32API_INIT_PROC(m_CfgMgr32, CM_Get_Device_Interface_ListW)
	{};

};