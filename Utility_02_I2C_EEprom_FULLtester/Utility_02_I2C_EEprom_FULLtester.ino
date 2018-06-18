/* Simple read & write to a 24LC256 256K EEPROM using the Wire library.
 * Addresses are ints - 0000-7FFF (32767) Data is bytes (8 bits x 32767 = 256K)
 * Functions for R/W of single byte or a page of bytes. Max page is 28 bytes.
 */

// Based on origininal code by bHogan user post at
// http://forum.arduino.cc/index.php/topic,18501.0.html

#include <Wire.h>
#define EEPROM_ADDR 0x50         // AT24C256 = 32768 bytes / characters (32k)
//  0x57  I2C Buss address of AT24C32 EEPROM with pins pulled high  AT24C32 = 4096 bytes / characters
//  EEprom address is 0x50 with pins pulled low (the 4k eerpom on the RTC boards)
#define startWriteAddressEEPROM 0  // you can just start this from 0
#define pageSize 32                // for the AT24c32 - check datasheet for other chips

//int cyclesWritten2EEPROM;
int currentPageStartAddress;
int nextPageStartAddress;  //nextPageStartAddress = currentPageStartAddress +/- pageSize; 

#define EEpromPageSize 32            // 32 bytes is page size for the AT24C32 & AT24C256 - can't move more than 30 bytes with wire.h anyway
uint16_t CurrentEEpromMemAddress = 0;// set to zero at the start of each cycle - must increment by steps of 32
char EEPROMBuffer[30]; //size=(PageSize - 2) This char array recieves a string of ascii from the pstring function 

void setup(){
  Wire.begin();                        // join I2C bus (address optional for master)
  
  //Changing I2C speed  http://www.gammon.com.au/forum/?id=10896
  //also http://electronics.stackexchange.com/questions/29457/how-to-make-arduino-do-high-speed-i2c
  //if (EEPROM_ADDRESS==0x50){  
  TWBR = 2;//for 400 kHz @ 8MHz CPU speed ONLY // AT24c256 ok @ 400kHz http://www.atmel.com/Images/doc0670.pdf  
  // see http://www.avrfreaks.net/forum/can-8mhz-clock-run-400khz-twi?skey=TWBR%208mhz 8mhz arduinos might not be capable of 400hz
  //TWBR = 12; // 200kHz mode @ 8MHz or 400kHz mode @ 16MHz CPU 
  //TWBR = 32;  // 100 kHz (default) @ 8MHz only 
  //}
  // AT24c256 ok to 400kHz http://www.atmel.com/Images/doc0670.pdf  
  // But the AT24c32 on the RTC board rated fro 100kHz  http://www.atmel.com/images/doc0336.pdf  
  // multispeed I2C scanner https://github.com/RobTillaart/Arduino/tree/master/sketches/MultiSpeedI2CScanner 
  // TWBR selects the division factor for the bit rate generator. The bit rate generator is a frequency divider which generates the SCL clock frequency in the Master modes.  
  // TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
  // SCL frequency = CPU_FREQUENCY / (16 + 2(TWBR) * (PrescalerValue)) //Prescalar=TWSR  // http://www.gammon.com.au/i2c    
  // for a CPU speed of 16000000 that will give:  TWBR =  ((16000000 / 100000) - 16) / 2 = 72 
  
  Serial.begin(9600);
  Serial.println("");Serial.flush();

//=======================test for 0x50==================================
    Serial.print("Testing EEprom at 0x");  Serial.println(EEPROM_ADDR,HEX);
  // TESTS FOR EACH FUNCTION BEGIN HERE
  Serial.print("Writing Test:");
  for (int i=0; i<20; i++){            // loop for first 20 slots
    i2c_eeprom_write_byte(EEPROM_ADDR,i,i+65);   // write address + 65 A or 97 a
    Serial.print(". ");
    delay(10);                         // NEED THIS DELAY!
  }
  Serial.println("");
  delay(500);
  Serial.print("Reading Test:");
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
  //could change this to 16 bytes in my code, arbitrarily chose 100 as the memory address here

  Serial.print("Reading Page Test:");
  i2c_eeprom_read_buffer( EEPROM_ADDR, 100, PageRead, 16);
  for (int i=0; i<16; i++){
    Serial.print(PageRead[i]);    // display the array read
    Serial.print(" ");
  }
//=====================test for 0x57======================================
    Serial.println("");  Serial.println("");
    Serial.print("Testing 4K EEprom at 0x57 (on RTC module)");
  // TESTS FOR EACH FUNCTION BEGIN HERE
  Serial.print("Writing Test:");
  for (int i=0; i<20; i++){            // loop for first 20 slots
    i2c_eeprom_write_byte(0x57,i,i+65);   // write address + 65 A or 97 a
    Serial.print(". ");
    delay(10);                         // NEED THIS DELAY!
  }
  Serial.println("");
  delay(500);
  Serial.print("Reading Test: ");
  for (int i=0; i<20; i++){            // loop for first 20 slots
    Serial.write(i2c_eeprom_read_byte(0x57, i));
    Serial.print(" ");
  }

  for (int i=0; i<16; i++){            // zero both arrays for before running test
    PageData[i] = 0;    
    PageRead[i] = 0;
  }
  Serial.println("");
  for (int i=0; i<16; i++) {
    PageData[i] = i+33;
  }  // fill up array for next test with numbers starting at 33

    Serial.println("Writing Page Test: 16 numbers from 33-48");
  i2c_eeprom_write_page(0x57, 100, PageData, 16 ); // 28 bytes/page is max whole page is written at once here
  //could change this to 16 bytes in my code, arbitrarily chose 100 as the memory address here

  Serial.print("Reading Page Test: ");
  i2c_eeprom_read_buffer(0x57, 100, PageRead, 16);
  for (int i=0; i<16; i++){
    Serial.print(PageRead[i]);    // display the array read
    Serial.print(" ");
  }

    Serial.println(" ");
}  //================================== end of setup ====================================

void loop(){

for (int i=0; i<30; i++) {
    EEPROMBuffer[i] = 32;
  }  // fill up array with dots
  
byte bytebuffer;
  
//=======================LONG test for 0x57==================================

//this is a longish slow test... deliberately putting the eeprom through a series of read/write cycles
     Serial.println(" "); Serial.print("LONG test for 4K EEprom at 0x57(on RTC module)");
  // TESTS FOR EACH FUNCTION BEGIN HERE
  Serial.print("Filling ALL memory with blank spaces... (Normal FILL=~5sec) ");
  unsigned long start = millis();
  CurrentEEpromMemAddress=0; start = millis(); 
  for (int i=0; i<128; i++){            // loop in 32 byte pages
    Write_i2c_eeprom_page(0x57,CurrentEEpromMemAddress,EEPROMBuffer);
    //because TWI 32byte buffer uses 2bytes for mem address, this can only fill 30 char of the 32 byte page
    i2c_eeprom_write_byte(0x57,(CurrentEEpromMemAddress+30),32);   // these lines fill those last two characters
    i2c_eeprom_write_byte(0x57,(CurrentEEpromMemAddress+31),32); 
    CurrentEEpromMemAddress += EEpromPageSize; 
    //delay(10); // NEED THIS DELAY!
  }
  Serial.println("");
  Serial.print("(Normal FILL=~5sec) Fill loop took:");Serial.print(millis()-start);Serial.println("(ms)");
  Serial.println("");
  Serial.println("Now Reading & printing ALL eeprom memory: (Normal READback ~5sec) ");
  start = millis();
  for (int i=0; i<4096; i++){            //loop
    bytebuffer=i2c_eeprom_read_byte(0x57,i);
    Serial.write(bytebuffer);
    //Serial.print(i2c_eeprom_read_byte(0x57), i));
    //Serial.print(" ");
    if((i % 64)==0){Serial.println("");}  //modulo to add line breaks//remainder = dividend % divisor;
  }
  Serial.println("");
  Serial.println("If you see anything other than blank spaces, then there is a mem location error!");
  Serial.println("(Normal READback ~5sec) READ loop actually took:");Serial.print(millis()-start);Serial.print("(ms)");
  Serial.println("");

//=======================LONG test for 0x50==================================
// this is a longish slow test... deliberately putting the eeprom through a long series of read/write cycles

//this is a longish (30sec) slow test...deliberately putting the eeprom through a series of read/write cycles
     Serial.println(" "); Serial.println("LONG test for 32K EEprom at 0x50");
  
  Serial.print("Filling ALL memory with blank spaces... (normally ~20sec) ");
  CurrentEEpromMemAddress=0; start = millis(); 
  for (int i=0; i<1024; i++){            // loop THROUGH THE ENTIRE MEMORY  1024(32-byte pages!)
    Write_i2c_eeprom_page(0x50,CurrentEEpromMemAddress,EEPROMBuffer);
    //because TWI 32byte buffer uses 2bytes for mem address, this can only fill 30 char of the 32 byte page
    i2c_eeprom_write_byte(0x50,(CurrentEEpromMemAddress+30),32);   // these lines fill those last two characters
    i2c_eeprom_write_byte(0x50,(CurrentEEpromMemAddress+31),32); 
    CurrentEEpromMemAddress += EEpromPageSize; 
    //delay(10); // NEED THIS DELAY!
  }
  Serial.println("");
  Serial.print("(normally ~20sec) Fill loop actually took:");Serial.print(millis()-start);Serial.println("(ms)");
  Serial.println("");
  Serial.println("Reading & printing ALL eeprom memory: normally ~35sec)");
  start = millis();
  for (uint16_t i=0; i<32768; i++){  //loop requires larger uint value -  32767 is max for normal int
    bytebuffer=i2c_eeprom_read_byte(0x50,i);
    Serial.write(bytebuffer);
    if((i % 64)==0){Serial.println("");}  //modulo to add line breaks//remainder = dividend % divisor;
  }
  Serial.println("");
  Serial.println("If you saw anything other than blank spaces, then there is a mem location error!");
  Serial.print("(normally ~35sec) READ loop took:");Serial.print(millis()-start);Serial.print("(ms)");
  Serial.println("");


 while(1) { }  //this while loop should stop the program
 delay(100000);
  
} // end of main loop //==============================


//====================================================================
// https://learn.sparkfun.com/tutorials/reading-and-writing-serial-eeproms
// Microchip 24LC1025 can store up to 128K! but it uses two separate "blocks" for the addressing

// Address is a page address, 6-bit (63). More and end will wrap around
// But data can be maximum of 28 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
  //void eeprom_read_block (void * pointer_ram , const void * pointer_eeprom , size_t n)
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddresspage >> 8)); // Address High Byte
  Wire.write((int)(eeaddresspage & 0xFF)); // Address Low Byte
  byte c;
  for ( c = 0; c < length; c++)  //this is the little loop that puts each character in 'data' char into the page
    Wire.write(data[c]);
  Wire.endTransmission();
  delay(10);                           // need some delay
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

// The functions listed below are not part of the eeprom tester, but are VERY handy to know about.
// I use them to buffer data in the I2C eeproms from Atmel like the AT24C32 that comes on many DS3231 RTC boards
// It's worth noting that writing a byte, and writing a page to these eeproms takes the same amount of power

// EEprom PAGE read & write functions
//===================================
// Address is a page address - But data can be maximum of 30 bytes
// The Arduino Wire library only has a 32 character buffer, so that is the maximun we can send using Arduino. 
// This buffer includes the two address bytes which limits our data payload to 30 bytes, but I have never gotten this routine to 
// work beyond 30 bytes of payload.
// The time overhead for page writes  = 5ms, whilst the overhead for writing individual bytes = 3.5ms

void Write_i2c_eeprom_page( uint8_t deviceAddress, uint16_t registerAddress_16bit, char* arrayPointer)
{
  uint8_t i = 0;
  Wire.beginTransmission(deviceAddress);
  Wire.write((byte)((registerAddress_16bit) >> 8));   // send the MSB of the address
  Wire.write((byte)((registerAddress_16bit) & 0xFF)); // send the LSB of the address
  do {
    Wire.write((byte) arrayPointer[i]);
    i++;
  }
  while (arrayPointer[i]);
  Wire.endTransmission();
  //digitalWrite(BLUE_PIN, HIGH);  //just an indication of the eeprom saving
  delay(6);  // data sheet says it takes 5ms for a page write of 32 bytes, whilst time for writing individual bytes = 3.5ms each byte
  //LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_ON); //sleeping for 15ms uses less power than delay(6);
  //digitalWrite(BLUE_PIN, LOW);
}

void Read_i2c_eeprom_page( uint8_t deviceAddress, uint16_t registerAddress_16bit, char* arrayPointer, uint8_t numOfChars)
{
  uint8_t i = 0;
  Wire.beginTransmission(deviceAddress);
  Wire.write((byte)(registerAddress_16bit >> 8));   // send the MSB of the address
  Wire.write((byte)(registerAddress_16bit & 0xFF)); // send the LSB of the address
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)deviceAddress, (uint8_t)(numOfChars - 1));
  while (Wire.available()) {
    arrayPointer[i++] = Wire.read(); //added brackets
  }
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
  delay(10);  // data sheet says 5ms
}
