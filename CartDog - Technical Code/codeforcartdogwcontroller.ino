//import all libraries for bluetooth, servo motors, gps sensor
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <TinyGPSPlus.h>
#include <Servo.h>

#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true  // If set to 'true' enables debug output
#define BLE_READPACKET_TIMEOUT         500   // Timeout in ms waiting to read a response

/* Create the bluefruit object with hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */

// SHARED SPI SETTINGS
// ----------------------------------------------------------------------------------------------
// The following macros declare the pins to use for HW and SW SPI communication.
// SCK, MISO and MOSI should be connected to the HW SPI pins on the Uno when
// using HW SPI.  This should be used with nRF51822 based Bluefruit LE modules
// that use SPI (Bluefruit LE SPI Friend).
// ----------------------------------------------------------------------------------------------
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4  // Optional but recommended, set to -1 if unused

//Adafruit Bluefruit LE Shield definition
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
// A small helper - doesn't run without it
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}
 
//GPS sensor definition
TinyGPSPlus gps; //tinygpsplus object to control cart gps sensor

//right and left servo motor definitions
Servo motor_r; //servo object to control right motor
Servo motor_l; //servo object to control left motor

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];

void setup() {
  //motor pin setup
  motor_r.attach(9); // attaches servo motor on pin 9 to the motor_r object
  motor_l.attach(10); // attaches servo motor on pin 10 to the motor_l object

  Serial.begin(115200); //set the baud rate of the serial to 115200, which is the baud rate that must be set for the bluetooth to work properly

  //initialize the adafruit shield
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }

  //wait for connection
  while (! ble.isConnected()) {
      delay(500);
  }
  Serial.println("----------------STARTTTTTTTT-------------");

  //Set Bluefruit to DATA mode
  ble.setMode(BLUEFRUIT_MODE_DATA);
}

//define ints for the speed of each motor
int right_speed, left_speed;

void loop() {
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  //break out of loop() if no button is pressed
  if(len == 0) return;

  // Button controller
  if (packetbuffer[1] == 'B') {
    //initialize values for the button number and whether a button is being pressed
    uint8_t buttnum = packetbuffer[2] - '0';
    bool pressed = packetbuffer[3] - '0';
    // Serial.print ("Button "); Serial.print(buttnum);
    while (pressed) {
      //if the forward/up button is pressed
      if (buttnum == 5) {
        Serial.println("moving forward");
        right_speed = 180;
        left_speed = 0;
      }

      //if the right button is pressed
      if (buttnum == 8) {
        Serial.println("moving right");
        right_speed = 90;
        left_speed = 0;
      }

      //if the backwards/down button is pressed
      if (buttnum == 6) {
        Serial.println("moving backward");
        right_speed = 0;
        left_speed = 180;
      }

      //if the left button is pressed
      if (buttnum == 7) {
        Serial.println("moving left");
        right_speed = 180;
        left_speed = 90;
      }

      //move in the direction set by the right and left speeds, corresponding to the button being pressed
      motor_r.write(right_speed);
      motor_l.write(left_speed);

      //read new values from the phone control pad
      len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
      buttnum = packetbuffer[2] - '0';
      pressed = packetbuffer[3] - '0';

    }
    //if the button is released, set the motors to stop moving
    motor_r.write(90);
    motor_l.write(90);
  }
  
}




