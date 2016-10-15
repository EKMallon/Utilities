#include <Wire.h>       // I2C lib needs 128 byte Serial buffer
#include <SPI.h>        // not used here, but needed to prevent a RTClib compile error....grrr!
#include <RTClib.h>     // Date, Time and Alarm functions by https://github.com/MrAlvin/RTClib based largely on https://github.com/jcw
#include <LowPower.h>   // https://github.com/rocketscream/Low-Power
#include <avr/sleep.h> 
//#include <avr/wdt.h>  // re-enable this if you switch away from the RS sleep libraries
#include <PString.h>    // from  http://arduiniana.org/
#include <SdFat.h>      // needs 512 byte buffer,  from https://github.com/greiman/SdFat
SdFat sd; /*Create the objects to talk to the SD card*/
SdFile file;

#define __AVR_ATmega1284P__  //so that the read internal Vcc routine works

//#define ECHO_TO_SERIAL   // for debugging  this takes about the same memory as doing the first order pressure calculations!

//const byte chipSelect = 10; //rockets
const byte chipSelect = 1; //MMotes

//sd card chip select 10 is taken on the Moteino Mega
//pins 10 & 11 are interrupt 0&1 on the MegaMote

//#define RTCPOWER_PIN 7 //rockets
#define RTCPOWER_PIN 14 //MegaMotes

// 3 color indicator LED pin connections
//#define RED_PIN 4  //rockets
//#define GREEN_PIN 5
//#define BLUE_PIN 6

// MMotes
#define RED_PIN 20  
#define GREEN_PIN 21
#define BLUE_PIN 22

byte READSENSOR_PIN = GREEN_PIN;

RTC_DS3231 RTC;                            //DS3231 will function with a VCC ranging from 2.3V to 5.5V
byte Alarmhour = 1;
byte Alarmminute = 1;
byte Alarmday = 1;                         //only used for sub second alarms
byte Alarmsecond = 1;                      //only used for sub second alarms
byte INTERRUPT_PIN = 2;                    // RTC SQW is soldered to this pin on the arduino
volatile boolean clockInterrupt = false;
char CycleTimeStamp[ ]= "0000/00/00 00:00"; //16 characters without seconds!
//byte Startday;

void setup() {
  
    #ifdef RTCPOWER_PIN
    pinMode(RTCPOWER_PIN, HIGH);
    digitalWrite(RTCPOWER_PIN, OUTPUT);// driving this high supplies power to the RTC Vcc pin while arduino is awake
    #endif
  
    Serial.begin(57600);  //do I need this when running on batteries?  could go to 115200 for faster serial coms?
 
    Wire.begin();
    
    RTC.begin();

    // check RTC
    //**********
    //clearClockTrigger(); //stops RTC from holding the interrupt low if system reset just occured
     RTC.turnOffAlarm(1);
  
  
      //get the SD card ready
  //**********************
  pinMode(chipSelect, OUTPUT);  //make sure that the default chip select pin is set to output, even if you don't use it 

  #ifdef ECHO_TO_SERIAL
  Serial.print(F("Init SD card...")); 
  #endif

  // Initialize SdFat or print a detailed error message and halt
  // Use SPI_HALF_SPEED to behave like the native library. // change to SPI_FULL_SPEED for more performance.  
  // SPI (Serial Peripheral Interface): http://www.gammon.com.au/spi  
  // SD card latencies vary randomly between 41ms to 300ms and block Arduino in the meantime: http://forum.arduino.cc/index.php?topic=256084.0
    if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
    #ifdef ECHO_TO_SERIAL
    Serial.println(F("Could NOT initialize SD Card"));Serial.flush();
    #endif
    error();
    } 

    #ifdef ECHO_TO_SERIAL
    Serial.flush();
    #endif
    
    for (int CNTR = 0; CNTR < 6; CNTR++) { //# of seconds of flashing red light on error = CNTR/2 with 250 ms delay
    digitalWrite(RED_PIN, HIGH);   
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON);                 
    digitalWrite(RED_PIN, LOW); 
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON);  
    }
    
  
  //}
  
}

void loop() {
 
    #ifdef ECHO_TO_SERIAL
    Serial.println(F("Made it to main loop - shutting down via error now"));Serial.flush();
    #endif
    
    error();
}


/**********************************************
 * ERROR HANDLER "Houston we have a problem..."
 ***********************************************/
// more advanced debugging: http://forum.arduino.cc/index.php?topic=203282.0
void error() 
{   
  
#ifdef __AVR_ATmega1284P__
 pinMode(0,INPUT_PULLUP);// pullup the onewire pin
        pinMode(12,INPUT_PULLUP);// pullup the onewire pin
            pinMode(13,INPUT_PULLUP);// pullup the onewire pin
                pinMode(18,INPUT_PULLUP);// pullup the onewire pin
                    pinMode(19,INPUT_PULLUP);// pullup the onewire pin
                        pinMode(2,INPUT_PULLUP);// pullup the onewire pin
                            pinMode(3,INPUT_PULLUP);// pullup the onewire pin
                                pinMode(4,INPUT_PULLUP);// pullup the onewire pin
`#endif
     
     if (file.isOpen()) {
      file.close();
    }
    
    #ifdef ECHO_TO_SERIAL
    Serial.flush();
    #endif
    
    #if defined RTCPOWER_PIN
    pinMode(RTCPOWER_PIN, INPUT);    //stop sourcing or sinking current
    digitalWrite(RTCPOWER_PIN, LOW); // driving this LOW FORCES to the RTC to draw power from the coin cell
    #endif
  

     for (int CNTR = 0; CNTR < 5; CNTR++) { //# of seconds of flashing red light on error = CNTR/2 with 250 ms delay
    digitalWrite(RED_PIN, HIGH);   
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON);                 
    digitalWrite(RED_PIN, LOW); 
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON); 
    digitalWrite(BLUE_PIN, HIGH);   
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON);                 
    digitalWrite(BLUE_PIN, LOW); 
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON);
    digitalWrite(GREEN_PIN, HIGH);   
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON);                 
    digitalWrite(GREEN_PIN, LOW); 
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_ON);  
    }
  
  //}
  #if defined RTCPOWER_PIN
  pinMode(RTCPOWER_PIN, INPUT);    //stop sourcing or sinking current
  digitalWrite(RTCPOWER_PIN, LOW); // driving this LOW FORCES to the RTC to draw power from the coin cell
  #endif
  #if defined POWERDOWN_PIN
  digitalWrite(POWERDOWN_PIN, HIGH);// driving this pin high shuts down the system if pololu power switch attached
  delay(10);// just in case it takes time to power down...
  #endif
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON); //if pololu shutdown does not happen go to sleep, leave BODon
//LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //if pololu shutdown does not happen go to sleep, leave BODon
  // or use:  sleepNwait4WDT(); //if you go to sleep without FIRST setting the wdt, this is sleep forever!

}
