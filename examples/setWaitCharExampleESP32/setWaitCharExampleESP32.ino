/*
  setWaitCharExampleESP32.ino - Example file to demonstrate SevenSeg74HC595 class setWaitChar() related methods
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

// Display instances creation
// Creating an instances of the display, the only parameters needed are
// the pins that will be connected to the display module usually marked as:
// SCLK
// RCLK
// DIO
SevenSeg74HC595 myLedDispOne(sclk, rclk, dio);

void setup(){
}

void loop(){  
  myLedDispOne.begin();

//====================================>> First example
  //Print a message telling a "configuration" will take place
  testResult = myLedDispOne.print("JuSt");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print(4, false);
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("SeCS");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("to");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("CnFG");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);

  //Setting the wait() method to keep the display alive while configuring
  testResult = myLedDispOne.wait(250);
  // delay(4000);
  vTaskDelay(4000/portTICK_RATE_MS);

  //Stop de waiting displayed
  testResult = myLedDispOne.noWait();

//====================================>> Second example
  //Change de wait character to _
  testResult = myLedDispOne.print("chnG");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("char");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("to .");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.setWaitChar('.');
  testResult = myLedDispOne.wait();
  // delay(3000);
  vTaskDelay(3000/portTICK_RATE_MS);
  testResult = myLedDispOne.noWait();

//====================================>> Third example
  //Change de wait character to _
  testResult = myLedDispOne.print("chnG");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("char");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("to _");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.setWaitChar('_');  
  testResult = myLedDispOne.wait();
  // delay(3000);
  vTaskDelay(3000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.noWait();

//====================================>> Fourth example
  //Change de wait character to o
  testResult = myLedDispOne.print("chnG");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("char");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("to o");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.setWaitChar('o');  
  testResult = myLedDispOne.wait();
  // delay(3000);
  vTaskDelay(3000/portTICK_RATE_MS);

  //Stop de waiting displayed
  testResult = myLedDispOne.noWait();
  
//====================================>> Fifth example
  //Change de wait character to 8
  testResult = myLedDispOne.print("chnG");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("char");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("to 8");
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.setWaitChar('8');  
  testResult = myLedDispOne.wait();
  // delay(3000);
  vTaskDelay(3000/portTICK_RATE_MS);

  //Stop de waiting displayed
  testResult = myLedDispOne.noWait();  

//====================================>> Ending examples, make general cleanup and resetting values
  //Set back the waiting rate to the original value
  testResult = myLedDispOne.setWaitRate(250);
  testResult = myLedDispOne.setWaitChar('-');
  
  testResult = myLedDispOne.print("the");
  // delay(2000);
  vTaskDelay(2000/portTICK_RATE_MS);
  
  testResult = myLedDispOne.print("End");
  // delay(2000);
  vTaskDelay(2000/portTICK_RATE_MS);

  //Stop and disengage the display from the ISR... and this is the end of the loop()
  myLedDispOne.clear();
  myLedDispOne.stop();
  // delay(1000);
  vTaskDelay(1000/portTICK_RATE_MS);
}