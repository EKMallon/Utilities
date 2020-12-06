/* 
 Use this sketch to reset the DS1307 & DS3231 Real Time Clocks. It will grab
 the computer's current time during compilation and send that value
 to the real time clock. 
 
 Afterwards, immediately upload a DIFFERENT sketch to the Arduino
 so that it doesn't try to constantly reset the clock when it 
 powers up the next time. If it does reset, the Arduino will reset 
 the clock with the old compile time stamp, which will be out of
 date.
*/

#include <Wire.h>
#include <SPI.h>  // not used here, but needed to prevent a RTClib compile error
#include "RTClib.h"   // library from https://github.com/MrAlvin/RTClib/

RTC_DS1307 RTC;     // Setup an instance of DS1307 naming it RTC
#define DS3231_I2C_ADDRESS 0x68
#define DS3231_CONTROL_REG 0x0E

void setup () {
    pinMode(13, OUTPUT);
    Wire.begin(); // Start the I2C
    RTC.begin();  // Init RTC
    RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date are set to date and time on your computer at compiletime

    enableRTCAlarmsonBackupBattery(); // only needed if you cut the Vcc pin supplying power to the DS3231 chip to run clock from coincell
}

void loop () {   //flashing led tells you the time has been set
  digitalWrite(13, HIGH);   // turn the LED on 
  delay(250);              // wait for a 1/4second
  digitalWrite(13, LOW);    // turn the LED off 
  delay(250);              // wait for a 1/4second
}

//====================================================================================
// Enable Battery-Backed Square-Wave Enable on the DS3231 RTC module: 
/* Bit 6 (Battery-Backed Square-Wave Enable) of DS3231_CONTROL_REG 0x0E, can be set to 1 
 * When set to 1, it forces the wake-up alarms to occur when running the RTC from the back up battery alone. 
 * [note: This bit is usually disabled (logic 0) when power is FIRST applied]
 */
  void enableRTCAlarmsonBackupBattery(){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);              // Attention device at RTC address 0x68
  Wire.write(DS3231_CONTROL_REG);            // move the memory pointer to CONTROL_REGister
  Wire.endTransmission();                    // complete the ‘move memory pointer’ transaction
  Wire.requestFrom(DS3231_I2C_ADDRESS,1);                  // request data from register
  byte resisterData = Wire.read();           // byte from registerAddress
  bitSet(resisterData, 6);                   // Change bit 6 to a 1 to enable
  Wire.beginTransmission(DS3231_I2C_ADDRESS);              // Attention device at RTC address 0x68
  Wire.write(DS3231_CONTROL_REG);            // target the CONTROL_REGister
  Wire.write(resisterData);                  // put changed byte back into CONTROL_REG
  Wire.endTransmission();
  }
