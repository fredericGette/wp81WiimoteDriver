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

#define IOCTL_WIIMOTE_CONNECT CTL_CODE(WIIMOTE_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_WIIMOTE_READ CTL_CODE(WIIMOTE_DEVICE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define ARRAY_SIZE 1024

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
char OutputBuffer[100];

MainPage::MainPage()
{
	InitializeComponent();
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
	else if (b->Tag->ToString() == "Read")
	{
		Read();
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

void wp81WiimoteDriver::MainPage::Install()
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

void wp81WiimoteDriver::MainPage::Run()
{
	Windows::UI::Xaml::Window^ window = Window::Current;

	Log(window, LogsList, L"Calling device...");
	create_task([this, window]()
	{
		HANDLE hDevice = win32Api.CreateFileW(L"\\\\.\\WiimoteRawPdo", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			LogError(window, LogsList, L"Failed to open device! 0x%X", GetLastError());
			return;
		}

		DWORD returned;
		BOOL success = win32Api.DeviceIoControl(hDevice, IOCTL_WIIMOTE_CONNECT, nullptr, 0, nullptr, 0, &returned, nullptr);
		if (success)
		{
			LogSuccess(window, LogsList, L"OK");
		}
		else
		{
			LogError(window, LogsList, L"Failed to send IOCTL_WIIMOTE_CONNECT! 0x%X", GetLastError());
		}

		CloseHandle(hDevice);
	});
}

void wp81WiimoteDriver::MainPage::Read()
{
	Windows::UI::Xaml::Window^ window = Window::Current;

	Log(window, LogsList, L"Reading device...");
	create_task([this, window]()
	{
		HANDLE hDevice = win32Api.CreateFileW(L"\\\\.\\WiimoteRawPdo", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			LogError(window, LogsList, L"Failed to open device! 0x%X", GetLastError());
			return;
		}

		while (true) {
			DWORD returned;
			ZeroMemory(OutputBuffer, sizeof(OutputBuffer));
			BOOL success = win32Api.DeviceIoControl(hDevice, IOCTL_WIIMOTE_READ, nullptr, 0, &OutputBuffer, sizeof(OutputBuffer), &returned, nullptr);
			if (success)
			{
				debug(L"Device call succeeded!\n");
				debug(L"returned %d\n", returned);
				debug(L"[0x%02X 0x%02X 0x%02X 0x%02X]\n", ((BYTE *)OutputBuffer)[0], ((BYTE *)OutputBuffer)[1], ((BYTE *)OutputBuffer)[2], ((BYTE *)OutputBuffer)[3]);
				if (((BYTE *)OutputBuffer)[3] != 0)
				{
					UIButton(true);
				}
				else
				{
					UIButton(false);
				}
			}
			else
			{
				debug(L"Device call failed!\n");
				Log(window, LogsList, L"-");
			}
		}

		CloseHandle(hDevice);
	});
}

void MainPage::UIButton(boolean flag) {
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, flag]()
	{
		if (flag)
		{
			Left->Background = ref new SolidColorBrush(Windows::UI::Colors::Blue);
		}
		else
		{
			Left->Background = ref new SolidColorBrush(Windows::UI::Colors::Black);
		}
		
	}));
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
