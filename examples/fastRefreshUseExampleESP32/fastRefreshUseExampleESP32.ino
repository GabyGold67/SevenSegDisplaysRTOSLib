/*
  fastRefreshUseExampleESP32.ino - Example file to demonstrate TM74HC595LedTube class fastRefresh() related methods
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
bool testResult{};

//Display instance creation
// Creating the instance of the display, the only parameters needed are
// the pins that will be connected to the display module, usually marked as:
// SCLK
// RCLK
// DIO
SevenSeg74HC595 myLedDisp(sclk, rclk, dio);

void setup(){
  //myLedDisp.begin();  //This example refreshes the display without the use of a timer interrupt
  testResult = myLedDisp.print("GabY");
}

void loop()
{
  myLedDisp.fastRefresh();
  // delay(6);   //This value is just to demonstrate the display tolerates keeping the data visible and have some time before starts to show a ghosting image
  vTaskDelay(8 / portTICK_PERIOD_MS);   //This value is just to demonstrate the display tolerates keeping the data visible and have some time before starts to show a ghosting image
}
