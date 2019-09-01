#include <Arduino.h>

#include <UIPEthernet.h> // Used for Ethernet

// **** ETHERNET SETTING ****
// Arduino Uno pins: 10 = CS, 11 = MOSI, 12 = MISO, 13 = SCK
// Ethernet MAC address - must be unique on your network - MAC Reads T4A001 in hex (unique in your network)
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };
// For the rest we use DHCP (IP address and such)

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
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // no point in carrying on, so do nothing forevermore:
      for (;;);
    }

    // print your local IP address:
    printIPAddress();
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
