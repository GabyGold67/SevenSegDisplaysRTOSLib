/*
  waitExampleESP32.ino - Example file to demonstrate SevenSeg74HC595 class wait() related methods
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

void setup()
{

}

void loop()
{  
  myLedDispOne.begin();

//====================================>> First example
  //Print a message telling a "configuration" will take place
  testResult = myLedDispOne.print("JuSt");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);

  testResult = myLedDispOne.print(6, false);
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("SeCS");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("to");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("CnFG");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  //Setting the wait() method to keep the display alive while configuring
  testResult = myLedDispOne.wait();
  //delay(6000);
  vTaskDelay(6000/portTICK_PERIOD_MS);
  
  //Stop de waiting displayed
  testResult = myLedDispOne.noWait();

//====================================>> Second example
  testResult = myLedDispOne.print("note");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("tHIS");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("CASE");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  //Trying to print while the display is in wait condition makes the message
  //printed to be immediately overwritten. The .wait() method must be stopped
  //to resume the usual display by a .noWait(method)
  testResult = myLedDispOne.wait(600);
  testResult = myLedDispOne.print("cant");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("Prnt");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("till");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("StOP");
  //delay(1000);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  
  //Stop de waiting displayed
  testResult = myLedDispOne.noWait();
  
  //Repeat the .print() executed while waiting...
  testResult = myLedDispOne.print("cant");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("Prnt");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("till");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("StOP");
  //delay(3000);
  vTaskDelay(3000/portTICK_PERIOD_MS);
  
//====================================>> Third example
  //Now normal display by other methods can be resumed
  //Show message by print() to the display
  testResult = myLedDispOne.print("LetS");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("do");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("thiS");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("FStr");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  //Setting the wait() method to keep the display alive with a new rate
  testResult = myLedDispOne.wait(100);
  //delay(5000);
  vTaskDelay(5000/portTICK_PERIOD_MS);
  
  //Stop de waiting displayed
  testResult = myLedDispOne.noWait();

//====================================>> Ending examples, make general cleanup and resetting values
  //Set back the waiting rate to the original value
  testResult = myLedDispOne.setWaitRate(250);
  
  testResult = myLedDispOne.print("the");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  testResult = myLedDispOne.print("End");
  //delay(2000);
  vTaskDelay(2000/portTICK_PERIOD_MS);
  
  //Stop and disengage the display from the ISR... and this is the end of the loop()
  myLedDispOne.clear();
  myLedDispOne.stop();
  //delay(1000);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  
}