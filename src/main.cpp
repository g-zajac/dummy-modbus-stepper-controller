#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h> // Used for Ethernet
#include <Modbus.h>
#include <ModbusIP.h>

const int ledPin = 5;
//Modbus Registers Offsets (0-9999)
const int SERVO_HREG = 100;

//ModbusIP object
ModbusIP mb;
long ts;

// Set Port to 502
EthernetServer server = EthernetServer(502);

// **** ETHERNET SETTING ****
// Arduino Uno pins: 10 = CS, 11 = MOSI, 12 = MISO, 13 = SCK
// Ethernet MAC address - must be unique on your network - MAC Reads T4A001 in hex (unique in your network)
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };
// The IP address for the shield
// byte ip[] = { 10, 0, 10, 211 };
void setup() {

  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  mb.config(mac);
  mb.addHreg(SERVO_HREG, 0);
}

void loop() {
    mb.task();
    Serial.print("received modbus: ");
    Serial.println(mb.Hreg(SERVO_HREG));
    delay(200);

    switch (Ethernet.maintain())
     {
       case 1:
         //renewed fail
         Serial.println("Error: renewed fail");
         break;

       case 2:
         //renewed success
         Serial.println("Renewed success");

         //print your local IP address:
         printIPAddress();
         break;

       case 3:
         //rebind fail
         Serial.println("Error: rebind fail");
         break;

       case 4:
         //rebind success
         Serial.println("Rebind success");

         //print your local IP address:
         printIPAddress();
         break;

       default:
         //nothing happened
         break;

}
