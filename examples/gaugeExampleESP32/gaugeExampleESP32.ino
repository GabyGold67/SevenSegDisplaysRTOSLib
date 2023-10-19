/*
  gaugeExampleESP32.ino - Example file to demonstrate SevenSeg-74HC595 class gauge() and blink() related methods
  Created by Gabriel D. Goldman, May, 2023.
  Updated by Gabriel D. Goldman, October, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
*/
#include <Arduino.h>
#include <SevenSeg-74HC595.h>

//Pin connection for Display
// Pin connected to DS of 74HC595 AKA DIO
const uint8_t dio {4}; // // Board pin # of ESP32 WROOM-32 uPesy dev Board
// Pin connected to ST_CP of 74HC595 AKA RCLK
const uint8_t rclk {2}; // Board pin # of ESP32 WROOM-32 uPesy dev Board
// Pin connected to SH_CP of 74HC595 AKA SCLK
const uint8_t sclk {0}; // Board pin # of ESP32 WROOM-32 uPesy dev Board

//Set of variables and constants needed just for Demo purposes
bool testResultOne{};

// Display instances creation
// Creating an instances of the display, the only parameters needed are
// the pins that will be connected to the display module usually marked as:
// SCLK
// RCLK
// DIO
SevenSeg74HC595 myLedDispOne(sclk, rclk, dio);

void setup()
{

}

void loop()
{
    myLedDispOne.begin();

  //gauge() with a floating point argument, 0.0 <= value <= 1.0, representing a percentage, the four ranges are:
  // 0 <= 1st range <0.25
  //0.25 <= 2nd range < 0.50
  //0.50 <= 3rd range < 0.75
  //0.75 <= 4rd range <= 1.0
  //A character to show ahead of the value is an option, 
  //if not specified will be a blank space, in this case 'b', or any other representable char, for example:
  //b for battery, F for fuel, t for temperature, r for remaining...
  testResultOne = myLedDispOne.gauge(1.0, 'b');
  // delay(3000);
  vTaskDelay(3000 / portTICK_RATE_MS);

  //Reduce the ammount displayed to 65%
  testResultOne = myLedDispOne.gauge(0.65, 'b');
  // delay(3000);
  vTaskDelay(3000 / portTICK_RATE_MS);

  //Reduce the ammount displayed to 40%
  testResultOne = myLedDispOne.gauge(0.40, 'b');
  // delay(3000);
  vTaskDelay(3000 / portTICK_RATE_MS);

  //Reduce the ammount displayed to 20%
  testResultOne = myLedDispOne.gauge(0.20, 'b');
  // delay(3000);
  vTaskDelay(3000 / portTICK_RATE_MS);

  //Reduce the ammount displayed to 0% and start blinking to show the very low level
  testResultOne = myLedDispOne.gauge(0.0, 'b');
  testResultOne = myLedDispOne.blink(500);
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);
  //Set the blink rate faster
  testResultOne = myLedDispOne.setBlinkRate(350);
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);

  //Set the blink rate faster
  testResultOne = myLedDispOne.setBlinkRate(200);
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);

  //Set the blink rate to the fastest accepted value
  testResultOne = myLedDispOne.setBlinkRate(myLedDispOne.getMinBlinkRate());
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);
  
  //Stop the blinking
  myLedDispOne.noBlink();

  // Set up a display for a temperature reading, using the gauge() with an integer argument, 0 <= value <= 3,
  //Set to level 0
  testResultOne = myLedDispOne.gauge(0, 't');
  // delay(3000);
  vTaskDelay(3000 / portTICK_RATE_MS);

  //Make the display show a rise in the temperature to level 1
  testResultOne = myLedDispOne.gauge(1, 't');
  // delay(3000);
  vTaskDelay(3000 / portTICK_RATE_MS);
  
  //Make the display show a rise in the temperature to level 2
  testResultOne = myLedDispOne.gauge(2, 't');
  // delay(3000);
  vTaskDelay(3000 / portTICK_RATE_MS);
  
  //Make the display show a rise in the temperature to level 3, and set it to blink to show the high level of temperature
  testResultOne = myLedDispOne.gauge(3, 't');
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);
  testResultOne = myLedDispOne.blink(500);
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);
  
  //Set the blink rate faster
  testResultOne = myLedDispOne.setBlinkRate(350);
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);

  //Set the blink rate faster
  testResultOne = myLedDispOne.setBlinkRate(200);
  // delay(1500);
  vTaskDelay(1500 / portTICK_RATE_MS);
  
  //Set the blink rate to the fastest accepted value
  testResultOne = myLedDispOne.setBlinkRate(myLedDispOne.getMinBlinkRate());
  // delay(1000);
  vTaskDelay(1000 / portTICK_RATE_MS);
  
  //Stop the blinking
  myLedDispOne.noBlink();

  //Stop and disengage the display from the ISR... and this is the end of the loop()
  myLedDispOne.clear();
  myLedDispOne.stop();
   // delay(1000);
  vTaskDelay(1000 / portTICK_RATE_MS);

}