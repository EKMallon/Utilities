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
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.begin(); // Start the I2C
    RTC.begin();  // Init RTC
 
    RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date are set to date and time on your computer at compiletime
 
    clearClockTrigger();
 
    enableRTCAlarmsonBackupBattery(); // only needed if you cut the Vcc pin supplying power to the DS3231 chip to run clock from coincell
}

void loop () {   //flashing led tells you the time has been set
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
  delay(250);              // wait for a 1/4second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
  delay(250);              // wait for a 1/4second
}

//====================================================================================
// clearClockTrigger function needed to acheive low sleep currents with DS3231 Vcc Pin cut
void clearClockTrigger()          // from http://forum.arduino.cc/index.php?topic=109062.0
{
  Wire.beginTransmission(0x68);   //Tell devices on the bus we are talking to the DS3231
  Wire.write(0x0F);               //Tell the device which address we want to read or write
  Wire.endTransmission();         //Before you can write to and clear the alarm flag
  Wire.requestFrom(0x68,1);       //you have to read the flag first! Read one byte
  Wire.read();                    //Read one byte - we are not interest in actually using it
  Wire.beginTransmission(0x68);   //Tell devices on the bus we are talking to the DS3231 
  Wire.write(0x0F);               //Status Register: Bit 3: zero disables 32kHz, Bit 7: zero enables the main oscilator
  Wire.write(0b00000000);         //Bit1: zero clears Alarm 2 Flag (A2F), Bit 0: zero clears Alarm 1 Flag (A1F)
  Wire.endTransmission();
}

//====================================================================================
// Enable Battery-Backed Square-Wave Enable on the DS3231 RTC module: 
/* Bit 6 (Battery-Backed Square-Wave Enable) of DS3231_CONTROL_REG 0x0E, can be set to 1 
 * When set to 1, it forces the wake-up alarms to occur when running the RTC from the back up battery alone. 
 * [note: This bit is usually disabled (logic 0) when power is FIRST applied]
 */
  void enableRTCAlarmsonBackupBattery(){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);      // Attention device at RTC address 0x68
  Wire.write(DS3231_CONTROL_REG);                  // move the memory pointer to CONTROL_REGister
  Wire.endTransmission();                          // complete the ‘move memory pointer’ transaction
  Wire.requestFrom(DS3231_I2C_ADDRESS,1);          // request data from register
  byte resisterData = Wire.read();                 // byte from registerAddress
  bitSet(resisterData, 6);                         // Change bit 6 to a 1 to enable
  Wire.beginTransmission(DS3231_I2C_ADDRESS);      // Attention device at RTC address 0x68
  Wire.write(DS3231_CONTROL_REG);                  // target the CONTROL_REGister
  Wire.write(resisterData);                        // put changed byte back into CONTROL_REG
  Wire.endTransmission();
  }

