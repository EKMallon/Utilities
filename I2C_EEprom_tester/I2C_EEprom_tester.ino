// this script posted by BroHogan at the Arduino playground
// see Re: I2C Code for Serial EEPROMs at https://forum.arduino.cc/index.php?topic=85382.0

/* Simple read & write to a 24LC series EEPROM using the Wire library.
* Addresses are ints - 0000-7FFF (32767) Data is bytes (8 bits x 32767 = 256K)
* Functions for R/W of single byte or a page of bytes. Max page payload is 28 bytes.
* SETUP:           _ _                                 
* Arduino GND- A0-|oU |-Vcc  to Arduino Vcc
* Arduino GND- A1-|   |-WP   to GND for now. Set to Vcc for write protection.
* Arduino GND- A2-|   |-SCL  to Arduino 5
* Arduino GND-Vss-|   |-SDA  to Arduino 4
*                  ---       (A2, A1, A0 to GND for 1010000 (0x50) address.)
*                            If set to Vcc adds to address (1010,A2,A1,A0)
*/

#include <Wire.h>
#define EEPROM_ADDR 0x57   // I2C Buss address of the AT24C32 4K EEPROM on the DS3231 RTC module  (may also be 0x56!)

void setup(){
 Wire.begin();              // join I2C bus
 Serial.begin(9600);

 // TESTS FOR EACH FUNCTION BEGIN HERE
 Serial.println("Writing Test:");
 for (int i=0; i<20; i++){            // loop for first 20 slots
   i2c_eeprom_write_byte(EEPROM_ADDR,i,i+65);   // write address + 65 A or 97 a
   Serial.print(". ");
   delay(10);                         // NEED THIS DELAY!
 }
 Serial.println("");
 delay(500);
 Serial.println("Reading Test:");
 for (int i=0; i<20; i++){            // loop for first 20 slots
   Serial.write(i2c_eeprom_read_byte(EEPROM_ADDR, i));
   Serial.print(" ");
 }
 // setup for page tests . . .
 byte PageData[30];                   // array that will hold test data for a page
 byte PageRead[30];                   // array that will hold result of data for a page
 for (int i=0; i<30; i++){            // zero both arrays for next test
   PageData[i] = 0;    
   PageRead[i] = 0;
 }
 Serial.println("");
 for (int i=0; i<30; i++) PageData[i] = i+33;  // fill up array for next test char 33 = !

 Serial.println("Writing Page Test:");
 i2c_eeprom_write_page(EEPROM_ADDR, 100, PageData, 28 ); // 28 bytes/page is max

 Serial.println("Reading Page Test:");
 i2c_eeprom_read_buffer( EEPROM_ADDR, 100, PageRead, 28);
 for (int i=0; i<28; i++){
   Serial.write(PageRead[i]);    // display the array read
   Serial.print(" ");
 }
}

void loop(){
}

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
 int rdata = data;
 Wire.beginTransmission(deviceaddress);
 Wire.write((int)(eeaddress >> 8));    // Address High Byte
 Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
 Wire.write(rdata);
 Wire.endTransmission();
}

// Address is a page address, 6-bit (63). More and end will wrap around
// But data can be maximum of 28 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
 Wire.beginTransmission(deviceaddress);
 Wire.write((int)(eeaddresspage >> 8)); // Address High Byte
 Wire.write((int)(eeaddresspage & 0xFF)); // Address Low Byte
 byte c;
 for ( c = 0; c < length; c++)
   Wire.write(data[c]);
 Wire.endTransmission();
 delay(10);                           // need some delay
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
 byte rdata = 0xFF;
 Wire.beginTransmission(deviceaddress);
 Wire.write((int)(eeaddress >> 8));    // Address High Byte
 Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
 Wire.endTransmission();
 Wire.requestFrom(deviceaddress,1);
 if (Wire.available()) rdata = Wire.read();
 return rdata;
}

// should not read more than 28 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
 Wire.beginTransmission(deviceaddress);
 Wire.write((int)(eeaddress >> 8));    // Address High Byte
 Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
 Wire.endTransmission();
 Wire.requestFrom(deviceaddress,length);
 //int c = 0;
 for ( int c = 0; c < length; c++ )
   if (Wire.available()) buffer[c] = Wire.read();
}

