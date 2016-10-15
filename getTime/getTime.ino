// verify first to reduce your time lag on upload of the setTime sketch
// Arduino Trick: Double Upload Speed  https://www.sparkfun.com/news/1552  

#include <Wire.h>
#include <SPI.h>          // Not used here but required for this RTClib to compile properly
#include <RTClib.h>       // this is the library from https://github.com/MrAlvin/RTClib

RTC_DS1307 RTC;

void setup(void)
{
  Wire.begin();
  RTC.begin(); 
}

void loop(void)
{
  DateTime now = RTC.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
  
// unix time takes less memory than an ascii timestamp
  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("days"); 
  delay(1000);
}

