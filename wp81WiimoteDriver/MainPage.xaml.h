//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace wp81WiimoteDriver
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();


	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void InstallDrivers();
		void ConnectWiimote();
		void CopyFiles(std::stack<Platform::String^> fileNames);
		void RegisterDriver(Platform::String ^ driverName, Platform::String ^ driverDescription);
		void AddUpperFilter(Platform::String ^ filterName, Platform::String ^ targetDriver);
		void CheckTestSignedDriver();
		void UIButton(BYTE firstByte, BYTE secondByte);
		void DisconnectWiimote();
		void DetectWiimotes();
		void OnSelectDevice(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
