#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <winnt.h>

using namespace Platform;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;



#define LOG_INFO 0
#define LOG_SUCCESS 1
#define LOG_ERROR 2

void debug(WCHAR* format, ...)
{
	va_list args;
	va_start(args, format);

	WCHAR buffer[1000];
	_vsnwprintf_s(buffer, sizeof(buffer), format, args);

	OutputDebugStringW(buffer);

	va_end(args);
}

void Log(Windows::UI::Xaml::Window^ window, ListBox^ listBox, INT logType, Platform::String^ string)
{
	debug(L"window %p\n", window);
	debug(L"listBox %p\n", listBox);

	window->Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([string, listBox, logType]()
	{
		TextBlock^ tbx = ref new TextBlock();
		tbx->TextWrapping = TextWrapping::Wrap;
		tbx->Text = string;

		Border^ border;

		switch (logType) {
		case LOG_INFO:
			listBox->Items->Append(tbx);
			listBox->UpdateLayout();
			listBox->ScrollIntoView(tbx);
			break;
		case LOG_SUCCESS:
			tbx->Foreground = ref new SolidColorBrush(Windows::UI::Colors::LightGreen);
			listBox->Items->Append(tbx);
			listBox->UpdateLayout();
			listBox->ScrollIntoView(tbx);
			break;
		case LOG_ERROR:
			tbx->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Black);
			border = ref new Border();
			border->Background = ref new SolidColorBrush(Windows::UI::Colors::Red);
			border->Child = tbx;
			listBox->Items->Append(border);
			listBox->UpdateLayout();
			listBox->ScrollIntoView(border);
			break;
		}
	}));
}

Platform::String^ VarArgs2String(WCHAR* format, va_list args)
{
	wchar_t* wcharString = (wchar_t*)malloc(1024);
	Platform::String^ string;

	size_t sizeOfWcharString = 1024 / sizeof(wchar_t);
	_vsnwprintf_s(wcharString, sizeOfWcharString, _TRUNCATE, format, args);

	string = ref new String((wchar_t*)wcharString);

	free(wcharString);

	return string;
}

void Log(Windows::UI::Xaml::Window^ window, ListBox^ listBox, WCHAR* format, ...)
{
	va_list args;
	Platform::String^ string;

	va_start(args, format);
	string = VarArgs2String(format, args);
	va_end(args);

	Log(window, listBox, LOG_INFO, string);
}

void LogSuccess(Windows::UI::Xaml::Window^ window, ListBox^ listBox, WCHAR* format, ...)
{
	va_list args;
	Platform::String^ string;

	va_start(args, format);
	string = VarArgs2String(format, args);
	va_end(args);

	Log(window, listBox, LOG_SUCCESS, string);
}

void LogError(Windows::UI::Xaml::Window^ window, ListBox^ listBox, WCHAR* format, ...)
{
	va_list args;
	Platform::String^ string;

	va_start(args, format);
	string = VarArgs2String(format, args);
	va_end(args);

	Log(window, listBox, LOG_ERROR, string);
}