#include <Arduino.h>
#include <SPI.h>  //redundant?
#include <Ethernet.h>
#include "credentials.h"
// **** ETHERNET SETTING ****
// Ethernet MAC address - must be unique on your network - MAC Reads T4A001 in hex (unique in your network)
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };
// For the rest we use DHCP (IP address and such)

#include <AccelStepper.h>
// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define DIR_PIN 17
#define STEP_PIN 18
#define MOTOR_INTERFACE_TYPE 1
#define ENABLE 19

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(MOTOR_INTERFACE_TYPE, STEP_PIN, DIR_PIN);

EthernetClient ethClient;

void setup() {
  //teensy WIZ820io initialisation code
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);    // begin reset the WIZ820io
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);  // de-select WIZ820io
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);   // de-select the SD Card
  digitalWrite(9, HIGH);   // end reset pulse

  delay(1000);            //TODO remove in producition
  Serial.begin(9600);
  Serial.println("Serial initiated");
  // Check for Ethernet hardware present
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  // try to congifure using IP address instead of DHCP:
  Ethernet.begin(mac);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  Serial.println("stepper test");
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);
  stepper.setMaxSpeed(4000.0);
  stepper.setAcceleration(800.0);
}

void loop() {

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
         Serial.println(Ethernet.localIP());
         break;

       case 3:
         //rebind fail
         Serial.println("Error: rebind fail");
         break;

       case 4:
         //rebind success
         Serial.println("Rebind success");

         //print your local IP address:
         Serial.println(Ethernet.localIP());
         break;

       default:
         //nothing happened
         break;

     }

}
