# wp81WiimoteDriver
Windows Phone 8.1 driver for Nintendo Wiimote.

Pair a Wiimote with the pin "---".  
A filter driver will intercept this pin and replace it by the Bluetooth address of the wiimote as expected by it. 
Without pairing the Wiimote is disconnected by Windows after around 60 secondes.

**Work in progress**
- The Bluetooth address of the Wiimote is hardcoded in the driver.

Compilation requires Visual Studio 2015 with Windows Phone 8.1 support.

Execution requires Nokia Lumia 520 with Windows Phone 8.1 rooted with [WPinternals](https://github.com/ReneLergner/WPinternals).
