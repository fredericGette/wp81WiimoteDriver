//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Win32Api.h"
#include <initguid.h>
#include "Log.h"

#define WIIMOTE_DEVICE 0x8000

#define IOCTL_WIIMOTE_CONNECT CTL_CODE(WIIMOTE_DEVICE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WIIMOTE_READ CTL_CODE(WIIMOTE_DEVICE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WIIMOTE_DISCONNECT CTL_CODE(WIIMOTE_DEVICE, 0x802, METHOD_NEITHER, FILE_ANY_ACCESS)

using namespace wp81WiimoteDriver;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace concurrency;
using namespace Windows::UI::Core;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

Win32Api win32Api;
BOOL stopReading;

MainPage::MainPage()
{
	InitializeComponent();
}

void DetectWiimotes()
{
	BLUETOOTH_DEVICE_INFO* device_info = new BLUETOOTH_DEVICE_INFO;
	ZeroMemory(device_info, sizeof(BLUETOOTH_DEVICE_INFO));
	device_info->dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

	debug(L"\nFind remote bluetooth device...\n");

	BLUETOOTH_DEVICE_SEARCH_PARAMS device_search_params;
	ZeroMemory(&device_search_params, sizeof(device_search_params));
	device_search_params.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
	device_search_params.fReturnAuthenticated = true;
	device_search_params.fReturnRemembered = true;
	device_search_params.fReturnUnknown = false;
	device_search_params.fReturnConnected = true;
	device_search_params.fIssueInquiry = false;
	device_search_params.hRadio = NULL;

	HBLUETOOTH_DEVICE_FIND device_search = win32Api.BluetoothFindFirstDevice(&device_search_params, device_info);
	if (device_search == NULL) {
		debug(L"Error BluetoothFindFirstDevice: %d\n", GetLastError());
		throw "Wiimote not detected. Please check that the Wiimote is paired. Press buttons 1+2.";
	}

	do
	{
		debug(L"\tDevice found: %s\n", device_info->szName);
		debug(L"\tAuthenticated: %s (%d)\n", (device_info->fAuthenticated != FALSE) ? L"True" : L"False", device_info->fAuthenticated);
		debug(L"\tRemembered: %s (%d)\n", (device_info->fRemembered != FALSE) ? L"True" : L"False", device_info->fRemembered);
		debug(L"\tConnected: %s (%d)\n", (device_info->fConnected != FALSE) ? L"True" : L"False", device_info->fConnected);
		debug(L"\tDevice address: %I64u\n", device_info->Address.ullLong);
		debug(L"\n");
	} while (win32Api.BluetoothFindNextDevice(device_search, device_info));
}


/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter

	CheckTestSignedDriver();
	DetectWiimotes();
}

void MainPage::AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button^ b = (Button^)sender;
	if (b->Tag->ToString() == "Install")
	{
		InstallDrivers();
	}
	else if (b->Tag->ToString() == "ConnectWiimote")
	{
		ConnectWiimote();
	}
	else if (b->Tag->ToString() == "DisconnectWiimote")
	{
		DisconnectWiimote();
	}
}

DWORD appendMultiSz(WCHAR* src, WCHAR* dst)
{
	DWORD size = 0;
	WCHAR* s = src;
	WCHAR* d = dst;
	do
	{
		*d = *s;
		s++;
		d++;
		size++;
	} while (*s != L'\0');
	*d = L'\0';
	size++;
	return size;
}

void MainPage::InstallDrivers()
{
	RegisterDriver(L"wp81wiimote", L"WP81 Wiimote driver");
	// Set wiimote driver as an upper filter of BTHENUM
	// lumia 735 : System\\CurrentControlSet\\Enum\\BTH\\MS_BTHBRB\\5&36dde44&0&0
	// lumia 520 : System\\CurrentControlSet\\Enum\\BTH\\MS_BTHBRB\\5&2215169c&0&0
	AddUpperFilter(L"wp81wiimote", L"System\\CurrentControlSet\\Enum\\BTH\\MS_BTHBRB\\5&2215169c&0&0");

	RegisterDriver(L"wp81pairingfilter", L"WP81 Wiimote Pairing Filter driver");
	// Set wp81pairingfilter as an upper filter of BTHMINI
	// lumia 520 : System\\CurrentControlSet\\Enum\\SystemBusQc\\SMD_BT\\4&315a27b&0&4097
	AddUpperFilter(L"wp81pairingfilter", L"System\\CurrentControlSet\\Enum\\SystemBusQc\\SMD_BT\\4&315a27b&0&4097");

	Log(Window::Current, LogsList, L"Install/Update drivers...");

	std::stack<Platform::String ^> fileNames;
	fileNames.push(L"wp81pairingfilter.sys");
	fileNames.push(L"wp81wiimote.sys");
	CopyFiles(fileNames);
}

void MainPage::ConnectWiimote()
{
	Windows::UI::Xaml::Window^ window = Window::Current;

	Log(window, LogsList, L"Connecting Wiimote...");
	create_task([this, window]()
	{
		HANDLE hDevice = win32Api.CreateFileW(L"\\\\.\\WiimoteRawPdo", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			LogError(window, LogsList, L"Failed to open device! 0x%X", GetLastError());
			return;
		}

		DWORD returned;
		ULONGLONG bthAddr = 247284104376928;
		debug(L"bthAddr %I64u\n", bthAddr);
		BOOL success = win32Api.DeviceIoControl(hDevice, IOCTL_WIIMOTE_CONNECT, &bthAddr, 8, nullptr, 0, &returned, nullptr);
		if (success)
		{
			LogSuccess(window, LogsList, L"OK");
		}
		else
		{
			LogError(window, LogsList, L"Failed to send IOCTL_WIIMOTE_CONNECT! 0x%X", GetLastError());
		}

		Log(window, LogsList, L"Reading data...");

		BYTE* outputBuffer = (BYTE*)malloc(128);
		size_t outputBufferSize = 128;
		stopReading = FALSE;
		while (!stopReading) {
			DWORD returned;
			ZeroMemory(outputBuffer, outputBufferSize);
			BOOL success = win32Api.DeviceIoControl(hDevice, IOCTL_WIIMOTE_READ, nullptr, 0, outputBuffer, outputBufferSize, &returned, nullptr);
			if (success)
			{
				debug(L"returned %d\n", returned);
				debug(L"[0x%02X 0x%02X 0x%02X 0x%02X]\n", outputBuffer[0], outputBuffer[1], outputBuffer[2], outputBuffer[3]);
				UIButton(outputBuffer[2], outputBuffer[3]);
			}
			else
			{
				LogError(window, LogsList, L"Failed to send IOCTL_WIIMOTE_READ! 0x%X", GetLastError());
			}
		}
		Log(window, LogsList, L"... stop reading data.");

		free(outputBuffer);
		CloseHandle(hDevice);
	});
}


void MainPage::UIButton(BYTE firstByte, BYTE secondByte) {
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, firstByte, secondByte]()
	{
		if (firstByte & 0x01)
		{
			Left->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			Left->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}
		
		if (firstByte & 0x02)
		{
			Right->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			Right->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (firstByte & 0x04)
		{
			Down->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			Down->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (firstByte & 0x08)
		{
			Up->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			Up->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (firstByte & 0x10)
		{
			ButtonPlus->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			ButtonPlus->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (secondByte & 0x01)
		{
			Button2->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			Button2->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (secondByte & 0x02)
		{
			Button1->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			Button1->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}
		
		if (secondByte & 0x04)
		{
			ButtonB->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			ButtonB->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (secondByte & 0x08)
		{
			ButtonA->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			ButtonA->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (secondByte & 0x10)
		{
			ButtonMinus->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			ButtonMinus->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

		if (secondByte & 0x80)
		{
			ButtonHome->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			ButtonHome->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}

	}));
}

void MainPage::DisconnectWiimote()
{
	Windows::UI::Xaml::Window^ window = Window::Current;

	Log(window, LogsList, L"Disconnecting wiimote...\nBriefly press a button of the Wiimote in order to finish reading data.");
	stopReading = TRUE;

	create_task([this, window]()
	{
		HANDLE hDevice = win32Api.CreateFileW(L"\\\\.\\WiimoteRawPdo", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			LogError(window, LogsList, L"Failed to open device! 0x%X", GetLastError());
			return;
		}

		DWORD returned;
		BOOL success = win32Api.DeviceIoControl(hDevice, IOCTL_WIIMOTE_DISCONNECT, nullptr, 0, nullptr, 0, &returned, nullptr);
		if (success)
		{
			LogSuccess(window, LogsList, L"OK");
			Log(window, LogsList, L"Press the power button of the Wiimote until the led is off.");
		}
		else
		{
			LogError(window, LogsList, L"Failed to send IOCTL_WIIMOTE_DISCONNECT! 0x%X", GetLastError());
		}

		CloseHandle(hDevice);
	});
}

void MainPage::CopyFiles(std::stack<Platform::String ^> fileNames) {

	Windows::UI::Xaml::Window^ window = Window::Current;

	if (fileNames.empty())
	{
		Log(window, LogsList, L"You can now reboot the phone to start the drivers.");
		return;
	}

	Platform::String^ fileName = fileNames.top();
	fileNames.pop();

	debug(L"%ls\n", fileName->Data());

	Log(window, LogsList, L"-Update %s...", fileName->Data());

	Uri^ uri = ref new Uri(L"ms-appx:///Payload/" + fileName);
	create_task(StorageFile::GetFileFromApplicationUriAsync(uri)).then([this, window, fileName, fileNames](task<StorageFile^> t)
	{
		StorageFile ^storageFile = t.get();
		Platform::String^ filePath = storageFile->Path;
		debug(L"FilePath : %ls\n", filePath->Data());
		Platform::String ^ newFileName = L"C:\\windows\\system32\\drivers\\" + fileName;
		if (!win32Api.CopyFileW(filePath->Data(), newFileName->Data(), FALSE))
		{
			LogError(window, LogsList, L"CopyFileW error: %d (32=ERROR_SHARING_VIOLATION)", GetLastError());
		}
		else
		{
			debug(L"File copied\n");
			LogSuccess(window, LogsList, L"OK");
			CopyFiles(fileNames);
		}
	});
}

void MainPage::RegisterDriver(Platform::String ^ driverName, Platform::String ^ driverDescription)
{
	Log(Window::Current, LogsList, L"Create/Update driver %s in registry... ", driverName->Data());
	
	HKEY HKEY_LOCAL_MACHINE = (HKEY)0x80000002;
	DWORD retCode;

	HKEY servicesKey = {};
	retCode = win32Api.RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services", 0, KEY_ALL_ACCESS, &servicesKey);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegOpenKeyExW : %d", retCode);
		return;
	}

	HKEY driverKey = {};
	retCode = win32Api.RegCreateKeyExW(servicesKey, driverName->Data(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &driverKey, NULL);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegCreateKeyExW '%s': %d", driverName, retCode);
		return;
	}

	retCode = win32Api.RegSetValueExW(driverKey, L"Description", NULL, REG_SZ, (BYTE*)driverDescription->Data(), (driverDescription->Length()+1) * sizeof(WCHAR));
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegSetValueExW 'Description': %d", retCode);
		return;
	}

	retCode = win32Api.RegSetValueExW(driverKey, L"DisplayName", NULL, REG_SZ, (BYTE*)driverName->Data(), (driverName->Length() + 1) * sizeof(WCHAR));
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegSetValueExW 'DisplayName': %d", retCode);
		return;
	}

	PBYTE ValueData = new BYTE[256];
	*(PDWORD)ValueData = 1; // Normal: If the driver fails to load or initialize, startup proceeds, but a warning message appears.
	retCode = win32Api.RegSetValueExW(driverKey, L"ErrorControl", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegSetValueExW 'ErrorControl': %d", retCode);
		return;
	}

	*(PDWORD)ValueData = 3; // SERVICE_DEMAND_START (started by the PlugAndPlay Manager)
	retCode = win32Api.RegSetValueExW(driverKey, L"Start", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegSetValueExW 'Start': %d", retCode);
		return;
	}

	*(PDWORD)ValueData = 1; // 	A kernel-mode device driver
	retCode = win32Api.RegSetValueExW(driverKey, L"Type", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegSetValueExW 'Type': %d", retCode);
		return;
	}

	retCode = win32Api.RegCloseKey(driverKey);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegCloseKey '%s': %d", driverName, retCode);
		return;
	}

	retCode = win32Api.RegCloseKey(servicesKey);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegCloseKey 'servicesKey': %d", retCode);
		return;
	}

	LogSuccess(Window::Current, LogsList, L"OK");
}

void MainPage::AddUpperFilter(Platform::String ^ filterName, Platform::String ^ targetDriver)
{
	Log(Window::Current, LogsList, L"Add %s as Upper Filter... ", filterName->Data());

	HKEY HKEY_LOCAL_MACHINE = (HKEY)0x80000002;
	DWORD retCode;

	WCHAR *newValueData = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 5000);
	DWORD newValueDataSize = 0;
	newValueDataSize += appendMultiSz((WCHAR*)filterName->Data(), newValueData);
	newValueDataSize++; // add final \0

	HKEY pdoKey = {};
	retCode = win32Api.RegOpenKeyExW(HKEY_LOCAL_MACHINE, targetDriver->Data(), 0, KEY_ALL_ACCESS, &pdoKey);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegOpenKeyExW : %d", retCode);
		return;
	}

	retCode = win32Api.RegSetValueExW(pdoKey, L"UpperFilters", NULL, REG_MULTI_SZ, (BYTE*)newValueData, newValueDataSize * 2);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegSetValueExW 'UpperFilters': %d", retCode);
		return;
	}

	retCode = win32Api.RegCloseKey(pdoKey);
	if (retCode != ERROR_SUCCESS)
	{
		LogError(Window::Current, LogsList, L"Error RegCloseKey 'pdoKey': %d", retCode);
		return;
	}

	LogSuccess(Window::Current, LogsList, L"OK");
}

void MainPage::CheckTestSignedDriver()
{
	Log(Window::Current, LogsList, L"Checking test-signed drivers...");

	HKEY HKEY_LOCAL_MACHINE = (HKEY)0x80000002;
	DWORD retCode;

	HKEY controlKey = {};
	retCode = win32Api.RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control", 0, KEY_ALL_ACCESS, &controlKey);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegOpenKeyExW : %d\n", retCode);
		return;
	}

	WCHAR ValueName[16383]; // buffer for value name
	DWORD ValueType;
	PBYTE ValueData = new BYTE[32767];

	DWORD i = 0;
	do
	{
		DWORD ValueNameSize = 16383;
		DWORD ValueDataSize = 32767;
		retCode = win32Api.RegEnumValueW(controlKey, i,
			ValueName,
			&ValueNameSize,
			NULL,
			&ValueType,
			ValueData,
			&ValueDataSize);

		debug(L"retCode %d Value name: %s\n", retCode, ValueName);

		if (wcscmp(L"SystemStartOptions", ValueName) == 0)
		{
			debug(L"Value: %s\n", ValueData);
			if (wcsstr((WCHAR*)ValueData, L"TESTSIGNING"))
			{
				debug(L"OK\n");
				LogSuccess(Window::Current, LogsList, L"OK");
			}
			else
			{
				LogError(Window::Current, LogsList, L"Failed\nPlease enable test-signed drivers to load!!");
			}
		}

		i++;
	} while (retCode == ERROR_SUCCESS);
}

