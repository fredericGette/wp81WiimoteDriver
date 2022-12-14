//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Win32Api.h"

#define WIIMOTE_DEVICE 0x8000

#define IOCTL_WIIMOTE_TEST CTL_CODE(WIIMOTE_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

Win32Api win32Api;

MainPage::MainPage()
{
	InitializeComponent();
}

void debug(WCHAR* format, ...)
{
	va_list args;
	va_start(args, format);

	WCHAR buffer[1000];
	_vsnwprintf_s(buffer, sizeof(buffer), format, args);

	OutputDebugStringW(buffer);

	va_end(args);
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter

}

void wp81WiimoteDriver::MainPage::AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button^ b = (Button^)sender;
	if (b->Tag->ToString() == "Install")
	{
		Install();
	}
	else if (b->Tag->ToString() == "Run")
	{
		Run();
	}
}

void wp81WiimoteDriver::MainPage::Install()
{
	TextTest->Text = L"Create driver WP81Wiimote in registry... ";

	HKEY HKEY_LOCAL_MACHINE = (HKEY)0x80000002;
	DWORD retCode;

	HKEY servicesKey = {};
	retCode = win32Api.RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services", 0, KEY_ALL_ACCESS, &servicesKey);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegOpenKeyExW : %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	HKEY wp81driverKey = {};
	retCode = win32Api.RegCreateKeyExW(servicesKey, L"wp81wiimote", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &wp81driverKey, NULL);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegCreateKeyExW 'wp81wiimote': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	PBYTE ValueData = new BYTE[256];

	ZeroMemory(ValueData, 256);
	wcscpy_s((WCHAR*)ValueData, 128, L"WP81 Wiimote driver");
	retCode = win32Api.RegSetValueExW(wp81driverKey, L"Description", NULL, REG_SZ, ValueData, 256);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'Description': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	ZeroMemory(ValueData, 256);
	wcscpy_s((WCHAR*)ValueData, 128, L"Wp81Wiimote");
	retCode = win32Api.RegSetValueExW(wp81driverKey, L"DisplayName", NULL, REG_SZ, ValueData, 256);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'DisplayName': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	*(PDWORD)ValueData = 1; // Normal: If the driver fails to load or initialize, startup proceeds, but a warning message appears.
	retCode = win32Api.RegSetValueExW(wp81driverKey, L"ErrorControl", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'ErrorControl': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	//*(PDWORD)ValueData = 1; // System: Loaded by I/O subsystem. Specifies that the driver is loaded at kernel initialization.
	*(PDWORD)ValueData = 3; // SERVICE_DEMAND_START (started by the PlugAndPlay Manager)
	retCode = win32Api.RegSetValueExW(wp81driverKey, L"Start", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'Start': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	*(PDWORD)ValueData = 1; // 	A kernel-mode device driver
	retCode = win32Api.RegSetValueExW(wp81driverKey, L"Type", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'Type': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	retCode = win32Api.RegCloseKey(wp81driverKey);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegCloseKey 'wp81wiimote': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	retCode = win32Api.RegCloseKey(servicesKey);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegCloseKey 'servicesKey': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	HKEY pdoKey = {};
	retCode = win32Api.RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Enum\\BTHENUM\\Dev_E0E751333260\\6&23f92770&0&BluetoothDevice_E0E751333260", 0, KEY_ALL_ACCESS, &pdoKey);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegOpenKeyExW : %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	ZeroMemory(ValueData, 256);
	wcscpy_s((WCHAR*)ValueData, 128, L"wp81wiimote");
	retCode = win32Api.RegSetValueExW(pdoKey, L"Service", NULL, REG_SZ, ValueData, 256);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'Service': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	retCode = win32Api.RegCloseKey(pdoKey);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegCloseKey 'pdoKey': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	TextTest->Text += L"OK\n";

	TextTest->Text += L"Install/Update driver...";

	Uri^ uri = ref new Uri("ms-appx:///Payload/wp81wiimote.sys");
	create_task(StorageFile::GetFileFromApplicationUriAsync(uri)).then([=](task<StorageFile^> t)
	{
		StorageFile ^storageFile = t.get();
		Platform::String^ filePath = storageFile->Path;
		debug(L"FilePath : %ls\n", filePath->Data());
		if (!win32Api.CopyFileW(filePath->Data(), L"C:\\windows\\system32\\drivers\\wp81wiimote.sys", FALSE))
		{
			debug(L"CopyFileW error: %d (32=ERROR_SHARING_VIOLATION)\n", GetLastError());
			create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
				ref new Windows::UI::Core::DispatchedHandler([=]()
			{
				TextTest->Text += L"Failed\n";
			})));
		}
		else
		{
			debug(L"File copied\n");
			create_task(Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
				ref new Windows::UI::Core::DispatchedHandler([=]()
			{
				TextTest->Text += L"OK\n";
			})));
		}
	});
}

void wp81WiimoteDriver::MainPage::Run()
{
	// Get handle of the first local bluetooth radio
	BLUETOOTH_FIND_RADIO_PARAMS radio_params;
	ZeroMemory(&radio_params, sizeof(radio_params));
	radio_params.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);
	HANDLE radio_handle;
	HBLUETOOTH_RADIO_FIND radio_search_result = win32Api.BluetoothFindFirstRadio(&radio_params, &radio_handle);
	if (radio_search_result == NULL)
	{
		debug(L"Error BluetoothFindFirstRadio: %d (259=ERROR_NO_MORE_ITEMS)\n", GetLastError());
		throw "Please check that Bluetooth is on.";
	}
	debug(L"radio_search_result=0x%08X radio_handle=0x%08X\n", radio_search_result, radio_handle);


	TextTest->Text += L"Calling device...";
	HANDLE hDevice = win32Api.CreateFileW(L"\\\\.\\WP81Wiimote", GENERIC_WRITE, FILE_SHARE_WRITE,
	nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		debug(L"Failed to open device.");
		TextTest->Text += L"Failed to open device.\n";
		return;
	}

	DWORD data = 0;

	DWORD returned;
	BOOL success = win32Api.DeviceIoControl(hDevice, IOCTL_WIIMOTE_TEST, &radio_handle, sizeof(radio_handle), nullptr, 0, &returned, nullptr);
	if (success)
	{
		debug(L"Device call succeeded!\n");
		TextTest->Text += L"succeeded!\n";
	}
	else
	{
		debug(L"Device call failed!\n");
		TextTest->Text += L"failed!\n";
	}

	CloseHandle(hDevice);
}