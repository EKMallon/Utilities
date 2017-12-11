// for reading the time from DS3231 RTC

#include <Wire.h>
#define RED_PIN 5
#define GREEN_PIN 6 
#define BLUE_PIN 7 
#define GROUND_PIN 4 

//DS3231 RTC variables
//====================
//#define RTCPOWER_PIN 7   // Assumes you are powering your DS3231 RTC from this pin. Even if you are not, it is harmless to leave this in unless you have something else connected to digital pin 7. 
#define DS3231_ADDRESS     0x68        //=104 dec
#define DS3231_STATUS_REG  0x0F
#define DS3231_CONTROL_REG 0x0E
#define DS3231_TMP_UP_REG  0x11

uint8_t t_second;   //= sec;  used in RTC_DS3231_getTime()
uint8_t t_minute;   //= min;
uint8_t t_hour;     //= hour;
uint8_t t_day;      //= day;
uint8_t t_month;    //= month;
uint16_t t_year;    //= year //yOff = raw year; //need to add 2000
uint32_t eTime;     //epoch time converted to unixtime

//This RTC used binary coded decimal format
static uint8_t bcd2bin (uint8_t val) {
  return val - 6 * (val >> 4);
}
static uint8_t bin2bcd (uint8_t val) {
  return val + 6 * (val / 10);
}

void setup(void)
{
  //pinMode(RTCPOWER_PIN, OUTPUT);   //some of my units use this pin for powering the RTC, need to set high for I2c coms.
  //digitalWrite(RTCPOWER_PIN, HIGH);
  pinMode(GROUND_PIN, OUTPUT);   //some of my units use this pin as the Ground for the indicator LED
  digitalWrite(GROUND_PIN, LOW);
  Wire.begin();
  Serial.begin(9600);
}

void loop(void)
{
  RTC_DS3231_getTime();
  Serial.print(t_year, DEC);
  Serial.print('/');
  Serial.print(t_month, DEC);
  Serial.print('/');
  Serial.print(t_day, DEC);
  Serial.print(' ');
  Serial.print(t_hour, DEC);
  Serial.print(':');
  Serial.print(t_minute, DEC);
  Serial.print(':');
  Serial.println(t_second, DEC);
  //TimeStamps can be created with sprintf(CycleTimeStamp, "%04d/%02d/%02d %02d:%02d",t_year,t_month,t_day,t_hour,t_minute);

  eTime = RTC_DS3231_unixtime(); // unix time takes less memory than an ascii timestamp
  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(eTime);  //Converting etime to Excel dates with =CELL/(60*60*24)+"1/1/1970"
  Serial.print("s = ");
  Serial.print(eTime / 86400L);
  Serial.println("days"); 
  digitalWrite(GREEN_PIN,HIGH);
  delay(500);
  digitalWrite(GREEN_PIN,LOW);
  delay(500);
}


// ============================================================================================================
// RTC functions
// ============================================================================================================
// these functions are from a fork of JeeLab's fantastic real time clock library for Arduino
// released into the public domain by Jeelabs https://github.com/jcw/rtclib & Ladyada & Eric Ayars, etc.

void RTC_DS3231_getTime()
{
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 7);
  t_second = bcd2bin(Wire.read() & 0x7F);
  t_minute = bcd2bin(Wire.read());
  t_hour = bcd2bin(Wire.read());
  Wire.read();
  t_day = bcd2bin(Wire.read());
  t_month = bcd2bin(Wire.read());
  t_year = bcd2bin(Wire.read()) + 2000;
  return;
}


//functions for UNIXTIME only:
//----------------------------
const uint8_t days_in_month [12] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//#define SECONDS_FROM_1970_TO_2000 946684800
uint32_t RTC_DS3231_unixtime() {
  uint32_t t;
  uint16_t days = date2days(t_year, t_month, t_day);
  t = time2long(days, t_hour, t_minute, t_second);
  t += 946684800;  // add # seconds from 1970 to 2000 which we took away with y -= 2000

  return t;
}

// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000)
    y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(days_in_month + i - 1);
  if (m > 2 && (y % 4 == 0)) // modulo checks (if is LeapYear) add extra day
    ++days;
  return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24L + h) * 60 + m) * 60 + s;
}

// Also see Andy Wickert's splice of those libraries at: https://github.com/NorthernWidget/DS3231
// also see https://github.com/sleemanj/DS3231_Simple/blob/master/DS3231_Simple.cpp for an alternate library
// also see https://github.com/mizraith/RTClib
// or https://github.com/akafugu/ds_rtc_lib for more DS3231 specific libs
// http://www.rinkydinkelectronics.com/library.php
// https://github.com/MajicDesigns/MD_DS3231/blob/master/src/MD_DS3231.cpp
// https://github.com/JChristensen/DS3232RTC wrapper for the time.h lib
// alternate unixtime calculation: https://github.com/rodan/ds3231
// see http://tronixstuff.com/2014/12/01/tutorial-using-ds1307-and-ds3231-real-time-clock-modules-with-arduino/

// Arduino Trick: Double Upload Speed  https://www.sparkfun.com/news/1552  ??
