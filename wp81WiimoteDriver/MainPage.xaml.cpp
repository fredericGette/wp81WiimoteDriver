//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Win32Api.h"
#include <initguid.h>

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

	TextTest->Text += L"Install/Update drivers...\n";

	std::stack<Platform::String ^> fileNames;
	fileNames.push(L"wp81pairingfilter.sys");
	fileNames.push(L"wp81wiimote.sys");
	CopyFiles(fileNames);
}

void wp81WiimoteDriver::MainPage::Run()
{

	TextTest->Text += L"Calling device...";
	create_task([this]()
	{
		HANDLE hDevice = win32Api.CreateFileW(L"\\\\.\\WiimoteRawPdo", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			debug(L"Failed to open device.");
			UIConsoleAddText(L"Failed to open device.\n");
			return;
		}

		DWORD returned;
		BOOL success = win32Api.DeviceIoControl(hDevice, IOCTL_WIIMOTE_CONNECT, nullptr, 0, nullptr, 0, &returned, nullptr);
		if (success)
		{
			debug(L"Device call succeeded!\n");
			UIConsoleAddText(L"succeeded!\n");
		}
		else
		{
			debug(L"Device call failed!\n");
			UIConsoleAddText(L"failed!\n");
		}

		CloseHandle(hDevice);
	});
}

void wp81WiimoteDriver::MainPage::Read()
{

	TextTest->Text += L"Reading device...";
	create_task([this]()
	{
		HANDLE hDevice = win32Api.CreateFileW(L"\\\\.\\WiimoteRawPdo", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			debug(L"Failed to open device.");
			UIConsoleAddText(L"Failed to open device.\n");
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
				UIConsoleAddText(L"-");
			}
		}

		CloseHandle(hDevice);
	});
}

void MainPage::UIConsoleAddText(Platform::String ^ text) {
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, text]()
	{
		TextTest->Text += text;
	}));
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

	if (fileNames.empty())
	{
		UIConsoleAddText(L"You can now reboot the phone to start the drivers.\n");
		return;
	}

	Platform::String^ fileName = fileNames.top();
	fileNames.pop();

	debug(L"%ls\n", fileName->Data());

	UIConsoleAddText(L"Update " + fileName + L"...");

	Uri^ uri = ref new Uri(L"ms-appx:///Payload/" + fileName);
	create_task(StorageFile::GetFileFromApplicationUriAsync(uri)).then([=](task<StorageFile^> t)
	{
		StorageFile ^storageFile = t.get();
		Platform::String^ filePath = storageFile->Path;
		debug(L"FilePath : %ls\n", filePath->Data());
		Platform::String ^ newFileName = L"C:\\windows\\system32\\drivers\\" + fileName;
		if (!win32Api.CopyFileW(filePath->Data(), newFileName->Data(), FALSE))
		{
			debug(L"CopyFileW error: %d (32=ERROR_SHARING_VIOLATION)\n", GetLastError());
			UIConsoleAddText(L"Failed\n");
		}
		else
		{
			debug(L"File copied\n");
			UIConsoleAddText(L"OK\n");
			CopyFiles(fileNames);
		}
	});
}

void MainPage::RegisterDriver(Platform::String ^ driverName, Platform::String ^ driverDescription)
{
	TextTest->Text += L"Create/Update driver "+driverName+L" in registry... ";
	
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

	HKEY driverKey = {};
	retCode = win32Api.RegCreateKeyExW(servicesKey, driverName->Data(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &driverKey, NULL);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegCreateKeyExW '%s': %d\n", driverName, retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	retCode = win32Api.RegSetValueExW(driverKey, L"Description", NULL, REG_SZ, (BYTE*)driverDescription->Data(), (driverDescription->Length()+1) * sizeof(WCHAR));
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'Description': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	retCode = win32Api.RegSetValueExW(driverKey, L"DisplayName", NULL, REG_SZ, (BYTE*)driverName->Data(), (driverName->Length() + 1) * sizeof(WCHAR));
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'DisplayName': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	PBYTE ValueData = new BYTE[256];
	*(PDWORD)ValueData = 1; // Normal: If the driver fails to load or initialize, startup proceeds, but a warning message appears.
	retCode = win32Api.RegSetValueExW(driverKey, L"ErrorControl", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'ErrorControl': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	*(PDWORD)ValueData = 3; // SERVICE_DEMAND_START (started by the PlugAndPlay Manager)
	retCode = win32Api.RegSetValueExW(driverKey, L"Start", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'Start': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	*(PDWORD)ValueData = 1; // 	A kernel-mode device driver
	retCode = win32Api.RegSetValueExW(driverKey, L"Type", NULL, REG_DWORD, ValueData, 4);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'Type': %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	retCode = win32Api.RegCloseKey(driverKey);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegCloseKey '%s': %d\n", driverName, retCode);
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

	TextTest->Text += L"OK\n";
}

void MainPage::AddUpperFilter(Platform::String ^ filterName, Platform::String ^ targetDriver)
{
	TextTest->Text += L"Add " + filterName + L" as Upper Filter... ";

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
		debug(L"Error RegOpenKeyExW : %d\n", retCode);
		TextTest->Text += L"Failed\n";
		return;
	}

	retCode = win32Api.RegSetValueExW(pdoKey, L"UpperFilters", NULL, REG_MULTI_SZ, (BYTE*)newValueData, newValueDataSize * 2);
	if (retCode != ERROR_SUCCESS)
	{
		debug(L"Error RegSetValueExW 'UpperFilters': %d\n", retCode);
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
}

void MainPage::CheckTestSignedDriver()
{
	TextTest->Text = "Checking test-signed drivers...";

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
				TextTest->Text += L"OK\n";
			}
			else
			{
				TextTest->Text += L"Failed\n";
				TextTest->Text += L"Please enable test-signed drivers to load!!\n";
			}
		}

		i++;
	} while (retCode == ERROR_SUCCESS);
}
