#include <Arduino.h>

// #include <SPI.h>
#include <Ethernet.h> // Used for Ethernet

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#define I2C_ADDRESS 0x3C

SSD1306AsciiAvrI2c oled;

// https://github.com/andresarmento/modbus-arduino
#include <Modbus.h>
#include <ModbusIP.h>

#include <Bounce2.h>
Bounce debouncer = Bounce(); // Instantiate a Bounce object

const int ledPin = 5;
const int buttonPin = 6;
bool alarmState = 0;

//Modbus Registers Offsets (0-9999)
const int HREG_ALARM_CODE = 40001;
const int HREG_P2P_DISTANCE = 40031;  //long
const int HREG_IMEDIATE_ABSOLUTE_POSITION  = 40007; //long

//ModbusIP object
ModbusIP mb;
long ts;
int analogIn = A6; // GPIO4
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

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  // oled.setFont(System5x7);
  oled.setFont(Arial14);
  oled.clear();
  oled.print("modbus v1.1");
  delay(1000);

  debouncer.attach(buttonPin,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer.interval(100); // Use a debounce interval of 25 milliseconds

  mb.config(mac);
  mb.addHreg(HREG_ALARM_CODE);
  mb.addHreg(HREG_P2P_DISTANCE);
  mb.addHreg(HREG_IMEDIATE_ABSOLUTE_POSITION);
}

void loop() {
    mb.task();
    debouncer.update(); // Update the Bounce instance

    if ( debouncer.fell() ) {  // Call code if button transitions from HIGH to LOW
     alarmState = !alarmState; // Toggle alarm state
     Serial.print("alarm state: ");
     Serial.println(alarmState);
     digitalWrite(ledPin, alarmState);
     mb.Hreg(HREG_ALARM_CODE, alarmState);
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Serial.print("updated HREG_P2P_DISTANCE: ");
      Serial.println(mb.Hreg(HREG_P2P_DISTANCE));
      oled.clear();
      oled.print("dist: "); oled.println(mb.Hreg(HREG_P2P_DISTANCE));
      Serial.print("analog input: ");
      position = analogRead(analogIn);
      Serial.println(position);
      oled.println("");
      oled.print("pos: "); oled.println(position);
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
