/* Simple read & write to a 24LC256 256K EEPROM using the Wire library.
 * Addresses are ints - 0000-7FFF (32767) Data is bytes (8 bits x 32767 = 256K)
 * Functions for R/W of single byte or a page of bytes. Max page is 28 bytes.
 */

// this EEprom test utiilty is based on an original post by user bHogan user post at http://forum.arduino.cc/index.php/topic,18501.0.html
// with various other useful bits of information added

#include <Wire.h>
#define EEPROM_ADDR 0x50          
// 0x57 I2C Bus address of AT24C32 4K EEPROM with pins pulled high
// EEprom address for that chip series is 0x50 with pins pulled low (which matches the AT24C256 from edragon)

//some helpful variables if you start using page writes:
#define pageSize 32                // this bytes per page limit imposed by the wire library
int cyclesWritten2EEPROM;          // to keep track of when your eeprom is full
int currentPageStartAddress=0;     // set to wherever you want the page writing to begin
int nextPageStartAddress=0;        // set to wherever you want the page writing to begin

void setup(){
  Wire.begin();                        // join I2C bus (address optional for master)
  
  // Changing I2C bus speed  http://www.gammon.com.au/forum/?id=10896
  // also http://electronics.stackexchange.com/questions/29457/how-to-make-arduino-do-high-speed-i2c
  // if (EEPROM_ADDRESS==0x50){  
  // TWBR = 2;//for 400 kHz @ 8MHz CPU speed ONLY // AT24c256 ok @ 400kHz http://www.atmel.com/Images/doc0670.pdf  
  // see http://www.avrfreaks.net/forum/can-8mhz-clock-run-400khz-twi?skey=TWBR%208mhz 8mhz arduinos might not be capable of 400hz
  // TWBR = 12; // 200kHz mode @ 8MHz or 400kHz mode @ 16MHz CPU 
  // TWBR = 32;  // 100 kHz (default) @ 8MHz only 
  // }
  // AT24c256 ok to 400kHz http://www.atmel.com/Images/doc0670.pdf  
  // But the AT24c32 on the RTC board rated fro 100kHz  http://www.atmel.com/images/doc0336.pdf  
  // multispeed I2C scanner https://github.com/RobTillaart/Arduino/tree/master/sketches/MultiSpeedI2CScanner 
  // TWBR selects the division factor for the bit rate generator. The bit rate generator is a frequency divider which generates the SCL clock frequency in the Master modes.  
  // TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
  // SCL frequency = CPU_FREQUENCY / (16 + 2(TWBR) * (PrescalerValue)) //Prescalar=TWSR  // http://www.gammon.com.au/i2c    
  // for a CPU speed of 16000000 that will give:  TWBR =  ((16000000 / 100000) - 16) / 2 = 72 
  
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
  byte PageData[16];                   // byte array that will hold test data for a page
  byte PageRead[16];                   // array that will hold result of data for a page
  for (int i=0; i<16; i++){            // zero both arrays for before running test
    PageData[i] = 0;    
    PageRead[i] = 0;
  }
  Serial.println("");
  for (int i=0; i<16; i++) {
    PageData[i] = i+33;
  }  // fill up array for next test with numbers starting at 33

    Serial.println("Writing Page Test: 16 numbers from 33-48");
  i2c_eeprom_write_page(EEPROM_ADDR, 100, PageData, 16 ); // 28 bytes/page is max whole page is written at once here
  //could change this to 16 bytes in my code, he arbitrarily chose 100 as the memory address here?

  Serial.println("Reading Page Test:");
  i2c_eeprom_read_buffer( EEPROM_ADDR, 100, PageRead, 16);
  for (int i=0; i<16; i++){
    Serial.print(PageRead[i]);    // display the array read
    Serial.print(" ");
  }
}

void loop(){
  //nothing here, only does the test once in the setup
}

// The functions listed below are not part of the eeprom tester, but are VERY handy to know about.
// I use them to buffer data in the I2C eeproms from Atmel like the AT24C32 that comes on many DS3231 RTC boards
// It's worth noting that writing a byte, and writing a page to these eeproms takes the same amount of power

// EEprom PAGE read & write functions
//===================================
// see http://www.hobbytronics.co.uk/eeprom-page-write
// Address is a page address - But data can be maximum of 28 bytes
// The Arduino Wire library only has a 32 character buffer, so that is the maximun we can send using Arduino. 
// This buffer includes the two address bytes which limits our data payload to 30 bytes, but I have never gotten this routine to 
// work beyond 28 bytes of payload.
// The time overhead for page writes  = 5ms, whilst the overhead for writing individual bytes = 3.5ms
void Write_i2c_eeprom_page( int deviceaddress, unsigned int eeaddress, char* data) 
{
  unsigned char i=0;
  unsigned int  address;
  address=eeaddress;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)((address) >> 8));   // MSB
  Wire.write((int)((address) & 0xFF)); // LSB
  do{ 
    Wire.write((byte) data[i]);
    i++;
  } 
  while(data[i]);  
  Wire.endTransmission(); 
  //delay(6);  // data sheet says 5ms for page write
  LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_ON);
}

void Read_i2c_eeprom_page( int deviceaddress, unsigned int eeaddress,char* data, unsigned int num_chars) 
{
  unsigned char i=0;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,(num_chars-1));
  while(Wire.available()) data[i++] = Wire.read();
}


// EEprom BYTE read & write functions
//===================================

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

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));    // Address High Byte
  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  Wire.write(rdata);
  Wire.endTransmission();
}

