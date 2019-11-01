#define FIRMWARE_VERSION 18
#include <Arduino.h>

// #include <SPI.h>
#include <Ethernet.h> // Used for Ethernet

// https://github.com/andresarmento/modbus-arduino
#include <Modbus.h>
#include <ModbusIP.h>

// adafruit OLED pinmap: data - sda (pin18), clk - scl (pin19), reset pin 20
// DATA - white (no1 from Teensy side) - pin 18
// CLK - yellow 2 -pin 19
// GND - black
// 5V - red
// TODO test if reset is needed for oled?
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
int progres_counter = 0;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     20 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <logo.h>

#include <TimeLib.h>
#include <Encoder.h>
/*
Rotary encoder wireing
(numbers of PCB connector pins, starting from teensy side top)
[connected to teensy pin]
[5](1)  grey  A  1         4 - Red - brown (4)[16]
[G](2)  white C- 2         5 - Green -
[6](3)  black B  3    X    6 - Switch - red (5)[15]
                          7 - Blue
                          8 - Common GND - orange (6)[GND]
*/
Encoder knob(6, 5);
int knob_position  = 0;

#include <Bounce2.h>
Bounce debouncer = Bounce(); // Instantiate a Bounce object

// TODO add build in led on GPIO13
const int buildInLed = 13;
const int alarmLedPin = 16;
const int knobButtonPin = 15;
bool alarmState = 0;

// Measuring Current Using ACS712
const int analogINpin = A9;
const float offset = 2.4;
float calibration = -0.100;
int noSamples = 100; //numbers of AD samples taken to count average
float voltage = 0;
float current = 0;

#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin 4
#define stepPin 3
#define motorInterfaceType 1
#define enable 2
int motor_position = 0;
int motor_position_new = 0;

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

//Modbus Registers Offsets (0-9999)
const int HREG_FIRMWARE_VERSION = 40109;
const int HREG_ALARM_CODE = 40001;
// const int HREG_P2P_DISTANCE = 40031;  //long
const int HREG_IMEDIATE_ABSOLUTE_POSITION  = 40007; //long
const int HREG_COMMAND_OPCODE = 40125;
const int HREG_RUNNING_CURRENT_H = 40051;
const int HREG_RUNNING_CURRENT_L = 40052;

//ModbusIP object
ModbusIP mb;
long ts;

// Set Port to 502
EthernetServer server = EthernetServer(502);

// **** ETHERNET SETTING ****
/*
RED (1) - PoE-
(2) - PoE +
(3) - GND
(4) - MI - MISO - pin 12
(5) - MO - MOSI - pin 11
(6) - SCK - pin 14
(7) - NSS - CS pin 10
(8) - RST
(9) - 5V+
(10) - GND
*/
// Arduino Uno pins: 10 = CS, 11 = MOSI, 12 = MISO, 13 = SCK
// Teensy 10 = CS(SS), 11 = MOSI, 12 = MISO, 14 (changed from 13 in code) = SCK
// Ethernet MAC address - must be unique on your network - MAC Reads T4A001 in hex (unique in your network)
//TODO change for MOONS mac address
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };
// The IP address for the shield
// byte ip[] = { 10, 0, 10, 211 };

unsigned long previousMillis = 0;
const long interval = 200;

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

void printDigits(byte digits){
 // utility function for digital clock display: prints colon and leading 0
 Serial.print(":");
 if(digits < 10)
   Serial.print('0');
 Serial.print(digits,DEC);
}

void time(long val){
int days = elapsedDays(val);
int hours = numberOfHours(val);
int minutes = numberOfMinutes(val);
int seconds = numberOfSeconds(val);

 // digital clock display of current time
 Serial.print(days,DEC);
 printDigits(hours);
 printDigits(minutes);
 printDigits(seconds);
 Serial.println();

}

void displayOnOled(String text, int row){
  display.setCursor(0,8*row);
  display.print("                ");
  display.setCursor(0,8*row);
  display.print(text);
  display.display();
}

void setup() {
  Serial.begin(9600);
  setTime(0); // start the clock
  pinMode(alarmLedPin, OUTPUT);
  digitalWrite(alarmLedPin, HIGH); // low = ON
  pinMode(buildInLed, OUTPUT);
  // delay(5000);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

  display.drawBitmap(
      (display.width()  - LOGO_WIDTH ) / 2,
      (display.height() - LOGO_HEIGHT) / 2,
      logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.display();
  delay(1000);
  display.clearDisplay();

  display.setTextColor(WHITE,BLACK);
  display.setTextSize(1);

  char buf_ver[12];
  sprintf(buf_ver, "fw ver: %d", FIRMWARE_VERSION);
  displayOnOled(buf_ver, 0);

  //free up pin 13 for builin LED
  SPI.setSCK(14);

  Serial.println("restarting ethernet module...");
  //teensy WIZ820io initialisation code
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);    // begin reset the WIZ820io
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);  // de-select WIZ820io
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);   // de-select the SD Card
  digitalWrite(9, HIGH);   // end reset pulse

  debouncer.attach(knobButtonPin,INPUT_PULLDOWN); // Attach the debouncer to a pin with pull down, switch connected to +3V3
  debouncer.interval(25); // Use a debounce interval of 25 milliseconds

  mb.config(mac);
  mb.addHreg(HREG_FIRMWARE_VERSION, FIRMWARE_VERSION);
  mb.addHreg(HREG_ALARM_CODE, 0);
  // mb.addHreg(HREG_P2P_DISTANCE);
  mb.addHreg(HREG_IMEDIATE_ABSOLUTE_POSITION);
  mb.addHreg(HREG_COMMAND_OPCODE, 0);
  mb.addHreg(HREG_RUNNING_CURRENT_H,0);
  mb.addHreg(HREG_RUNNING_CURRENT_L,0);

  // set stepper motor
  pinMode(enable, OUTPUT);
  digitalWrite(enable, LOW);
  stepper.setMaxSpeed(6400.0);  // 1.0 steps per second
  stepper.setAcceleration(3200.0);
}

void loop() {
    mb.task();
    debouncer.update(); // Update the Bounce instance

    if ( debouncer.fell() ) {  // Call code if button transitions from HIGH to LOW
     alarmState = !alarmState; // Toggle alarm state
     Serial.print("alarm state: ");
     Serial.println(alarmState);
     digitalWrite(alarmLedPin, !alarmState);
     // digitalWrite(buildInLed, alarmState);

    if (alarmState){
      mb.Hreg(HREG_ALARM_CODE, knob_position);
    } else {
      mb.Hreg(HREG_ALARM_CODE, 0);
    }
   }

   //TODO fix reading position x4?
   long knob_new_position;
   knob_new_position = knob.read();
   if (knob_new_position != knob_position){
     knob_position = knob_new_position;
   };

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // test
      motor_position = stepper.currentPosition() / 3200;  //  in revs
      motor_position_new = mb.Hreg(HREG_COMMAND_OPCODE); // in revs
      if ( motor_position != motor_position_new){
        stepper.runToNewPosition(motor_position_new * 3200); // 200 steps/rev , 16mikrostpes [200*1.8*16 ?]
      }

      char buf_up[16];
      sprintf(buf_up, "up: %02dd%02d:%02d:%02d",day()-1, hour(),minute(),second());
      displayOnOled(buf_up,1);

      char buf_mac[18];
      sprintf(buf_mac, "mac:%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      displayOnOled(buf_mac, 2);

      // update IP Address
      char buf_ip[16];
      sprintf(buf_ip, "ip: %d.%d.%d.%d", Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
      displayOnOled(buf_ip,3);

      char buf_reg1[18];
      sprintf(buf_reg1, "scl: %d", mb.Hreg(HREG_COMMAND_OPCODE));
      displayOnOled(buf_reg1, 4);

      char buf_reg2[18];
      sprintf(buf_reg2, "pos: %d", motor_position);
      displayOnOled(buf_reg2, 5);

      char buf_reg3[18];
      sprintf(buf_reg3, "alarm: %d", knob_position);
      displayOnOled(buf_reg3, 6);

      // Measuring Current Using ACS712
      // average AD reading
      float samplesVal = 0;
      float ascValue = 0;
      for (int i =0; i < noSamples; i++){
        ascValue = analogRead(analogINpin);
        samplesVal = samplesVal + ascValue;
        delay(3);
      }
      float voltage_avg = samplesVal / noSamples;
      voltage = ((3.3 / 1023.0) * voltage_avg) - 2.4 + calibration; // (max AD voltage / resolution) - offset
      current = voltage / 0.185;  // sesnsitivity 185mV
      int whole_cur = current; // casting
      int reminder_cur = (current - whole_cur) * 1000;
      // Serial.print("whole: "); Serial.println(whole);
      // Serial.print("reminder: "); Serial.println(reminder);
      mb.Hreg(HREG_RUNNING_CURRENT_H, whole_cur);
      mb.Hreg(HREG_RUNNING_CURRENT_L, reminder_cur);
      char buf_I[18];
      sprintf(buf_I, "I = %.3f", current);
      displayOnOled(buf_I, 7);

      if (progres_counter < 4) {
        display.setCursor(6*20,0);  //max 20 characters in line
        display.print(" ");
        display.setCursor(6*20,0);
        switch(progres_counter){
          case 0:
          display.print("-"); break;
          case 1:
          display.print("\\"); break;
          case 2:
          display.print("|"); break;
          case 3:
          display.print("/");break;
        }
        display.display();
        progres_counter++;
      } else progres_counter = 0;
    } // end of interval

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
