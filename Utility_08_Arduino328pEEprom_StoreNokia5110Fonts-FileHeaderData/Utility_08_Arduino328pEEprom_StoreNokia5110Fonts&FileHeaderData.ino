// This code stores a limited 5x8 "caps only" subset of the complete ASCII font for the nokia 5110 LCD screen
// along with a double size number font for display on a nokia 5110 LCD screen in the 1K internal eeprom of the 328p microprocessor used in some Arduinos
// the fonts take about 1/2 availiable space, and I store 521 characters of file header text in the remaining space
// this is "run once" utility, so I have not taken the time to optimize it's memory usage

// For details on how I connect the Nokia 5110 screens to an Arduino see:
// https://thecavepearlproject.org/2018/05/18/adding-the-nokia-5110-lcd-to-your-arduino-data-logger/

// For details on how I over-sample and calibrate thermistors on a 3.3v ProMini see:
// https://thecavepearlproject.org/2017/02/27/enhancing-arduinos-adc-resolution-by-dithering-oversampling/
// https://thecavepearlproject.org/2017/04/26/calibrating-oversampled-thermistors-with-an-arduino/

#include <EEPROM.h>
#define PADLENGTH 502     //set this to the # of eeprom charater/bytes you have left AFTER storing the LCD font data
char eepromString[PADLENGTH+1]={0}; //start with a null string
char charbuffer='A';      //a one character buffer for the eeprom loading & reading
int currentIntEEpromAddress=0; //memory location pointer


//================= store FONT data in arrays====================//
//the regular sized font set: 14 collumns x 6 rows fills the Nokia 5110 display
//Note: ASCII contains the entire upper & lower case font, but I only store a capital letters sub-set in the eeprom
//so I still have room for the big nums & text header data
const byte ASCII[][5] PROGMEM =
{
 {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -  //ascii decimal 45 //row 13 of array
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z //ascii decimal 90 // row 58 of array
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c ¥
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ←
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f →  (decimal 127)
};

//double size: 7 collumns x 3 rows fills the display  //this big #font requires 386 bytes of storage
// for details on how I use this "two-pass" big # font see: 
// https://thecavepearlproject.org/2018/05/18/adding-the-nokia-5110-lcd-to-your-arduino-data-logger/
const byte Big11x16numberTops[][11] PROGMEM = {
  0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00, // Code for char -
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // Code for char .
  0x00,0x00,0x00,0x00,0x00,0xC0,0xE0,0x78,0x1C,0x0F,0x03, // Code for char /
  0xFE,0xFF,0x03,0x03,0x03,0xC3,0x33,0x0B,0x07,0xFF,0xFE, // Code for char 0
  0x00,0x03,0x03,0x03,0x03,0xFF,0xFF,0x00,0x00,0x00,0x00, // Code for char 1
  0x03,0x83,0x83,0x83,0x83,0x83,0x83,0x83,0x83,0xFF,0xFE, // Code for char 2
  0x00,0x03,0x03,0x83,0x83,0x83,0x83,0x83,0x83,0xFF,0xFE, // Code for char 3
  0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0xFC,0xFC,0x00, // Code for char 4
  0x00,0x00,0xFF,0xFF,0x83,0x83,0x83,0x83,0x83,0x83,0x03, // Code for char 5
  0xFF,0xFF,0x03,0x03,0x03,0x03,0x03,0x03,0x07,0x06,0x00, // Code for char 6
  0x07,0x07,0x03,0x03,0x03,0x03,0x83,0x83,0xC3,0x7F,0x7F, // Code for char 7
  0x00,0x80,0xFF,0xFF,0x83,0x83,0x83,0xFF,0xFF,0x80,0x00, // Code for char 8
  0xFF,0xFF,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xFF,0xFF, // Code for char 9
};

const byte Big11x16numberBottoms[][11] PROGMEM = {
  0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00, // Code for char -
  0x00,0x00,0x00,0x00,0x0E,0x0E,0x0E,0x0E,0x00,0x00,0x00, // Code for char .
  0x00,0x60,0x78,0x3C,0x0F,0x03,0x01,0x00,0x00,0x00,0x00, // Code for char /
  0x3F,0x7F,0x70,0x6C,0x63,0x60,0x60,0x60,0x60,0x7F,0x3F, // Code for char 0
  0x00,0x60,0x60,0x60,0x60,0x7F,0x7F,0x60,0x60,0x60,0x60, // Code for char 1
  0x7F,0x7F,0x61,0x61,0x61,0x61,0x61,0x61,0x61,0x61,0x60, // Code for char 2
  0x00,0x60,0x60,0x61,0x61,0x61,0x61,0x61,0x61,0x7F,0x3F, // Code for char 3
  0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x7F,0x7F,0x03, // Code for char 4
  0x00,0x38,0x79,0x61,0x61,0x61,0x61,0x61,0x61,0x7F,0x3F, // Code for char 5
  0x7F,0x7F,0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x7F,0x7F, // Code for char 6
  0x00,0x00,0x00,0x00,0x00,0x7F,0x7F,0x01,0x00,0x00,0x00, // Code for char 7
  0x3F,0x7F,0x61,0x61,0x61,0x61,0x61,0x61,0x61,0x7F,0x3F, // Code for char 8
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x7F, // Code for char 9
};

//=================end FONT data====================//

void setup() {

/*
//This loop would store the complete font in ASCII[i][j] 
//filling the first 640 bytes (0-639) of the eeprom!
for(int i=0; i<127; i++){
  for(int j=0; i<5; j++){
  currentIntEEpromAddress=((j*5)+i);
  //LcdWriteData(pgm_read_byte(&ASCII[i][j]));
  EEPROM.write(currentIntEEpromAddress,pgm_read_byte(&ASCII[i][j]));
  }
}
*/
//However to save space I'm only storing 46 characters ie: (-)=array row 13 to capital(Z) =array row 59 
//This means I can only print capital letters to the LCD later but I'm willing to make that tradeoff for room to store the big numbers
//this takes 235 bytes of eeprom memory storage (addresses 0-234)

//===================
//Store the caps-only font in the eeprom
//You can only write one byte at a time to the eeprom storage
currentIntEEpromAddress=0;
for(int i=13; i<59; i++){
  for(int j=0; j<5; j++){
  currentIntEEpromAddress=(((i-13)*5)+j);
  //LcdWriteData(pgm_read_byte(&ASCII[i][j]));
  charbuffer=pgm_read_byte(&ASCII[i][j]);
  EEPROM.write(currentIntEEpromAddress,charbuffer);
  }
}

//====================
//Store the big number font "Tops" in the eeprom starting at eeprom memory address 235
//TOPs of big letters fills the next 143 bytes of the eeprom!
for(int i=0; i<13; i++){     // 13 characters in the array
  for(int j=0; j<11; j++){   // each font char is 11 bytes wide
  currentIntEEpromAddress=(235+(i*11)+j);  //note the 235 offset
  charbuffer=pgm_read_byte(&Big11x16numberTops[i][j]); 
  EEPROM.write(currentIntEEpromAddress,charbuffer);
  }
}

//======================
//Store the big number font "Bottoms" in the eeprom start at eeprom memory address 378
//BOTTOM row of big letters fills the next 143 bytes of the eeprom!
for(int i=0; i<13; i++){
  for(int j=0; j<11; j++){
  currentIntEEpromAddress=(378+(i*11)+j); //note the 378 offset
  charbuffer=pgm_read_byte(&Big11x16numberBottoms[i][j]); 
  EEPROM.write(currentIntEEpromAddress,charbuffer);
  }
}

// =========================================================
// Load header data as ascii text into eepromString variable
// =========================================================
// Including the "big numbers font" means that locations 0-520 of the internal eeprom memory are filled with LCD font data
// that leaves memory locations 521 to 1023 = 502 bytes of the internal eeprom storage free which is the PADLENGTH value
// I use this space to store file header data & sensor calibration constants, and reminders on how to do certain calculations
// edit the text to suit your own project, alternating lines with the "\r\n" to provide line returns in the data file
// \r is the ASCII character (carrier return (0xD), and \n is the ASCII character (line feed)(0xA) - \10 in decimal.
// many programs on Windows require BOTH \r\n , in linux, you need only \r and in Mac, you only need \n.
 
// WARNING: You have to have enough space in the first string to add the other strings or strcat will overflow
// DO NOT put more characaters into the eepromString than the space you have left in the eeprom (incl. the carridge returns)
// I don't have an error catch in this code for this so check your character count at sites like https://www.lettercount.com/
// snprintf() is a safer alternative to to strcat in that it prevents writing beyond the end of the destination string.  
// http://www.cplusplus.com/reference/cstdio/snprintf/  I just have not implemented that yet..
 
strcpy(eepromString," ");
strcat(eepromString, "\r\n"); //a carriage return in Excel
strcat(eepromString,"#198, The Cave Pearl Project");
strcat(eepromString, "\r\n");
strcat(eepromString,"Etime=(raw*iSec)/(86400)+\"1/1/1970\""); // NOTE: '\'= and escape character which lets you put special characters into the string
strcat(eepromString, "\r\n");
//strcat(eepromString,"Tres=SeriesRes/((32767(15bit)*(VccBGap/1100)/(RawADC))-1)"); //older builds with 15bit oversampling
strcat(eepromString,"Tres=(SeriesOhms)/((((65536*3)-1)/Raw16bitRead)-1)"); //16bit oversampling sampling
strcat(eepromString, "\r\n");
strcat(eepromString,"Temp C=[1/(A+(B*LN(Tres))+(C*(LN(Tres))^3))]-273.15"); //S&H thermistor calc
strcat(eepromString, "\r\n");
//strcat(eepromString,"682kseries/100Ktherm(16bitP32@1.1v),A=,-0.0002129864209,B=,0.0003327408854,C =,-0.0000001792470071");  //older builds w different series resistor
strcat(eepromString,"1M/100k@A7(16bitP32@1.1v),A=,-0.0003613129530,B=,0.0003479768279,C =,-0.0000001938681482");
strcat(eepromString, "\r\n");
//strcat(eepromString,"SDpow BJT,82k->D8,Bat=A0,coin-A1,RTC pinD7"); //older builds
strcat(eepromString,"Cal done:20180820, RTC=0.9719745x + 0.1984092");
strcat(eepromString, "\r\n");
strcat(eepromString,"EEprom:ASCII(-toZcaps)0-234+Big#font235-519,HeaderData@521");
strcat(eepromString, "\r\n");


// ===============================================================================================
// Pad out eepromString with enough blank spaces to fill ALL of the remaining space in the EEprom
// ===============================================================================================
int len = strlen(eepromString); //strlen does not inlude the nul terminator & sizeof does inlude the nul terminator in the number returned
memset(&eepromString[len],' ',PADLENGTH-len);
// now eepromString = the size of the remaining eeproms space

// ================================================
// store the contents of eepromString to the eeprom
// ================================================
//starting at the first memory location AFTER the font data we stored in startup
currentIntEEpromAddress=521;
for (int i = 0; i < PADLENGTH; i++){ 
    currentIntEEpromAddress=(521+i);
    if(currentIntEEpromAddress>1023){currentIntEEpromAddress=1023;}//error catch to make sure you don't write past last eeprom addr
    EEPROM.write(currentIntEEpromAddress, eepromString[i]); 
    // An EEPROM write takes 3.3 ms to complete. The EEPROM has a specified life of 100,000 write/erase cycles
}

Serial.begin(9600); 
}

void loop() {

//this loop will load and print the entire contents of the eeprom (including the fonts which print as random characters)
for (int j = 0; j < 1024; j++) {
     charbuffer = EEPROM.read(j);
     Serial.print(charbuffer);
    }

Serial.println(" ");Serial.println(" ");

//this loop will load and print ONLY the text information stored after the font data
//w comments showing how you could embed this when writing a data file to the SD card

// file.open(FileName, O_WRITE | O_APPEND);
for (int j = 521; j < 1023; j++) {
     charbuffer = EEPROM.read(j);
     Serial.print(charbuffer);
     // you could send this data to your SD card data file with:
     //file.print(charbuffer); //OR   file.write(charbuffer); 
    }
// file.close();

while(1); //this statement stops the main loop so it can only execute once - you dont want to burn out your eeprom with too many writes!
// http://arduino.land/FAQ/content/7/47/en/how-to-stop-an-arduino-sketch.html
// OR you could turn off interrupts and put the CPU into permanent sleep (until reset/power toggled): 
// cli();sleep_enable();sleep_cpu();

} // end of main loop

// other refs on using the eeprom can be found at:
// https://gist.github.com/smching/05261f11da11e0a5dc834f944afd5961  
// https://stackoverflow.com/questions/276827/string-padding-in-c
// https://arduino.stackexchange.com/questions/9349/possible-to-store-variables-directly-in-eeprom
// https://www.experts-exchange.com/questions/27852331/How-to-add-N-number-of-trailing-characters-to-a-C-char-array-or-C-String-object.html
