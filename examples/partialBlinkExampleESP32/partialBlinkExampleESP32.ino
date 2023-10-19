/*
  partialBlinkExampleESP32.ino - Example file to demonstrate SevenSeg-74HC595 class setBlinkMask() and blink() related methods
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
bool tstMask[4]{true, true, true, true};
bool testResult{};
const long testTime{3000};

//Display instance creation
// Creating the instance of the display, the only parameters needed are
// the pins that will be connected to the display module, usually marked as:
// SCLK
// RCLK
// DIO

SevenSeg74HC595 myLedDisp(sclk, rclk, dio);

void setup(){
  myLedDisp.begin();  //Attaching the display refresh to an ISR trough the begin() method
}

void loop()
{
  testResult = myLedDisp.print(3210);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  testResult = myLedDisp.blink();
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  tstMask[0] = true;
  tstMask[1] = false;
  tstMask[2] = false;
  tstMask[3] = false;
  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  tstMask[0] = false;
  tstMask[1] = true;
  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  tstMask[1] = false;
  tstMask[2] = true;
  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  tstMask[2] = false;
  tstMask[3] = true;
  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  testResult = myLedDisp.noBlink();
  myLedDisp.resetBlinkMask();
  testResult = myLedDisp.print("abcd");
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  testResult = myLedDisp.blink();
  tstMask[0] = true;
  tstMask[1] = false;
  tstMask[2] = false;
  tstMask[3] = false;

  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  tstMask[1] = true;
  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  tstMask[2] = true;
  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  tstMask[3] = true;
  myLedDisp.setBlinkMask(tstMask);
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
  testResult = myLedDisp.noBlink();
  myLedDisp.clear();
  // delay(testTime);
  vTaskDelay(testTime/portTICK_PERIOD_MS);
}
