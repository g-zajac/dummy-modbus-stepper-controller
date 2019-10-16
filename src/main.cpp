#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h> // Used for Ethernet
// https://github.com/andresarmento/modbus-arduino
#include <Modbus.h>
#include <ModbusIP.h>

const int ledPin = 5;
//Modbus Registers Offsets (0-9999)
const int SERVO_HREG = 30001;
const int HREG_IMEDIATE_ABSOLUTE_POSITION  = 40007; //long
//ModbusIP object
ModbusIP mb;
long ts;
int sensorPin = A6; // GPIO4
int position = 0;
// Set Port to 502
EthernetServer server = EthernetServer(502);

// **** ETHERNET SETTING ****
// Arduino Uno pins: 10 = CS, 11 = MOSI, 12 = MISO, 13 = SCK
// Ethernet MAC address - must be unique on your network - MAC Reads T4A001 in hex (unique in your network)
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };
// The IP address for the shield
// byte ip[] = { 10, 0, 10, 211 };

unsigned long previousMillis = 0;
const long interval = 1*1000;

void printIPAddress()
{
  Serial.println("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
  Serial.print("IP Address        : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask       : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Default Gateway IP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server IP     : ");
  Serial.println(Ethernet.dnsServerIP());
  Serial.println();
}

void setup() {

  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  mb.config(mac);
  mb.addHreg(SERVO_HREG, 0);
  mb.addHreg(HREG_IMEDIATE_ABSOLUTE_POSITION);
}

void loop() {
    mb.task();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Serial.print("updated modbus register: ");
      Serial.println(mb.Hreg(SERVO_HREG));
      Serial.print("reading position, analog input: ");
      position = analogRead(sensorPin);
      Serial.println(position);
      mb.Hreg(HREG_IMEDIATE_ABSOLUTE_POSITION, position);
    }

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
}
