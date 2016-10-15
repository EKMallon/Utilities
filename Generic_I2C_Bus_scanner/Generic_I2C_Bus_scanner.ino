//from  http://playground.arduino.cc/Main/I2cScanner
/*code written by:	 robtillaart
 This very simple sketch scans the I2C-bus for devices. 
 If a device is found, it is reported to the Arduino serial monitor.
 This sketch is the first step to get the I2C communication working.
 The sketch shows the 7-bit addresses of the found devices as hexadecimal values. 
 That value can be used for the "Wire.begin" function which uses the 7-bit address. 
 Some datasheets use the 8-bit address and some example sketches use decimal addresses.
 */

#include "Wire.h"
//#define RTCPOWER_PIN 14  //motinos
#define RTCPOWER_PIN 7  //rockets
byte start_address = 0;
byte end_address = 127;

void setup()
{   
  pinMode(RTCPOWER_PIN, HIGH);   //some of my units use this pin for powering the RTC, need to set high for I2c coms.
  byte rc;
  Wire.begin();

  Serial.begin(9600);
  Serial.println("\nI2C Scanner");
  Serial.flush();

  Serial.print("Scanning I2C bus from ");
  Serial.flush();
  Serial.print(start_address,DEC);  
  Serial.print(" to ");  
  Serial.print(end_address,DEC);
  Serial.println("...");

  for( byte addr  = start_address;
            addr <= end_address;
            addr++ ) {
      Wire.beginTransmission(addr);
    rc = Wire.endTransmission();

    if (addr<16) Serial.print("0");
    Serial.print(addr,HEX);
    if (rc==0) {
      Serial.print(" found!");
    } 
    else {
      Serial.print(" "); 
      Serial.print(rc); 
      Serial.print("     ");
    }
    Serial.print( (addr%8)==7 ? "\n":" ");
  }

  Serial.println("\n-------------------------------\nPossible devices:");
  Serial.flush();

  for( byte addr  = start_address;
            addr <= end_address;
            addr++ ) {
      Wire.beginTransmission(addr);
    rc = Wire.endTransmission();
    if (rc == 0) {
      Serial.print(addr,HEX); 
      Serial.print(" = ");
      switch (addr) {
      case 0x50: 
        Serial.println("AT24C32/AT24C64 - EEPROM"); 
        break;
      case 0x57: 
        Serial.println("AT24C32 - ADR pins high"); 
        break;
      case 0x19: 
        Serial.println("LM303 Accelerometer"); 
        break;
      case 0x68: 
        Serial.println("DS3231 / DS1307, also ITG3200 gyro"); 
        break;
      case 0x1E: 
        Serial.println("HMC5883L or LM303DLHC compass"); 
        break;
      case 0x40: 
        Serial.println("BMA180"); 
        break;
      case 0x1D: 
        Serial.println("ADXL345 or MMA7455L"); 
        break;
      case 0x29: 
        Serial.println("TSL2561 light sensor"); 
        break;
      case 0x3C: 
        Serial.println("SSD1306 OLED"); 
        break;
      case 0x48: 
        Serial.println("TMP102 temp _OR_ PN532 NFC/RFID"); 
        break;
      case 0x13: 
        Serial.println("VCNL4000 Proximity/Light sensor"); 
        break;
      case 0x18: 
        Serial.println("BMA250"); 
        break;
      case 0x76: 
        Serial.println("MS5803-02BA - Pin CSB high  OR MS561101BA CSB_HIGH "); 
        break;
      case 0x77: 
        Serial.println("MS5803-02BA - Pin CSB low OR BMP085"); 
        break;
        case 0x53: 
        Serial.println("ADXL345"); 
        break;
        case 0x38: 
        Serial.println("BMA020 2g accelerometer"); 
        break;  //  http://www.farnell.com/datasheets/1525403.pdf
      default: 
        Serial.println("Unknown"); 
        break;
      }
    }
  }

  Serial.println("\ndone");
}

// standard Arduino loop()
void loop()
{
  // Nothing to do here, so we'll just blink the built-in LED
  digitalWrite(13,HIGH); 
  delay(300);
  digitalWrite(13,LOW);  
  delay(300);
}

