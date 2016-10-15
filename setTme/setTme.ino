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

void setup () {
    pinMode(13, OUTPUT);
    Wire.begin(); // Start the I2C
    RTC.begin();  // Init RTC
    RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date are set to date and time on your computer at compiletime
}

void loop () {   //flashing led tells you the time has been set
  digitalWrite(13, HIGH);   // turn the LED on 
  delay(250);              // wait for a 1/4second
  digitalWrite(13, LOW);    // turn the LED off 
  delay(250);              // wait for a 1/4second
}
