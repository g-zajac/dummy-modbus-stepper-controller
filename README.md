# Dummy step servo controler with Modbus

## About The Project

This project is about mocking up an [Moons' Stepper Servo](https://www.moonsindustries.com/series/ss-series-step-servo-drives-a01030105) for developing a remote control web app with nodejs server.
Some modbus registers are mapped in teensy 3.2 controler acording to Moons documentation so the server-servo communication could be tested.

## Hardware
* Teensy 3.2
* ethernet w5100 module
* OLED display
* rotary encoder with RGB led
* MKS Servo 42 with closed loop module

![prototype](doc/dummy-modbus.png)

OLED on the device displays diagnostic informations i.e. IP address, values of some modbus registers - velocity, acceleration etc.
Rotary encoder is used for setting simulated error code.

## Built With
* [Atom](https://atom.io) - An amazing text editor
* [PlatformIO](https://platformio.org) - An ecosystem for embedded development

## License
This project is licensed under [MIT license](http://opensource.org/licenses/mit-license.php)

## Project status
- Prototype built and working.
- Firmware still under development.
