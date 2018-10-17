// SSD1306 128x64 OLED with shiftout
// Based primarily on Julian Ilett's SPI homebrew code for the Nokia 5110 LCD
// see https://www.youtube.com/watch?v=RAlZ1DHw03g&list=PLjzGSu1yGFjXWp5F4BPJg4ZJFbJcWeRzk
// combined with config settings from
// https://github.com/tardate/LittleArduinoProjects/blob/master/playground/OLED/SSD1306BareBack/SSD1306BareBack.ino
// Following init sequence from the data sheet https://github.com/GravCorp/DayOLED_SSD1306/blob/master/DayOLED_SSD1306.cpp

// NOTE to use the EEprom.read code show here you need to have pre-loaded these fonts into the 328p's internal eeprom space:
// there is a page describing this method in detail at: https://thecavepearlproject.org/2018/08/24/tutorial-using-the-arduinos-internal-eeprom-for-fonts-header-data/
// and an "EEprom loader" utility to use at https://github.com/EKMallon/Utilities/blob/master/Utility_08_Arduino328pEEprom_StoreNokia5110Fonts-FileHeaderData/Utility_08_Arduino328pEEprom_StoreNokia5110Fonts%26FileHeaderData.ino
// That example loads a caps only version of the 5x7 font, and a NUMBERS ONLY version of the bigger split font
// If you run this program without the fonts stored in the eeprom you will see nothing on the screen
// I've included the 5x7 & big numbers font(s) at the end of this file for completeness

#include <EEPROM.h>
char tmp[14];
unsigned long thisMicros = 0;
unsigned long lastMicros = 0;

//this code assumes the following OLED connections to a 3.3v Arduino Pro Mini
//============================================================================
// ARDUINO port -> connected to -> OLED screen terminal
// GND-->GND
// GND-->CS
// A0--> Vcc (power is provided by driving A0 high)
// A1--> DC
// A3--> DO (yellow)
// A2--> D1 (white)
// A0--> 10k resistor--> RES <--104 ceramic cap <--GND  [this provides a delayed high singal to the reset pin]

//=============WARNING============================================================================================================
// powering the screen this way usualy draws between 3-15mA for several lines of text
// be careful not to light up all the pixels at once(by using reverse fonts), as some screens may then
// exceed the max Arduino's pin current of 25mA! Also do not use these connections with a 5v Arduino or you will fry the 3.3v screen.
//=============WARNING============================================================================================================

#define PowerPin  A0    //A0 provides power to the screen
#define PIN_DC    A1    //blue
#define PIN_SDIN  A2    //white
#define PIN_SCLK  A3    //yellow
//#define PIN_CS    8   //not needed
//#define PIN_RESET 8   //not needed

//** dimensional settings
#define SSD1306_PIXEL_WIDTH             128
#define SSD1306_PIXEL_HEIGHT            64
#define SSD1306_PAGE_COUNT              8
#define SSD1306_PAGE_HEIGHT             SSD1306_PIXEL_HEIGHT / SSD1306_PAGE_COUNT
#define SSD1306_SEGMENT_COUNT           SSD1306_PIXEL_WIDTH * SSD1306_PAGE_HEIGHT
#define SSD1306_FONT_WIDTH              5

//** fundamental commands
#define SSD1306_SET_CONTRAST            0b10000001 // 0x81
#define SSD1306_CONTRAST_DEFAULT        0b01111111
#define SSD1306_DISPLAY                 0b10100100
#define SSD1306_DISPLAY_RESET           SSD1306_DISPLAY
#define SSD1306_DISPLAY_ALLON           SSD1306_DISPLAY | 0b01
#define SSD1306_DISPLAY_NORMAL          SSD1306_DISPLAY | 0b10
#define SSD1306_DISPLAY_INVERSE         SSD1306_DISPLAY | 0b11
#define SSD1306_DISPLAY_SLEEP           SSD1306_DISPLAY | 0b1110
#define SSD1306_DISPLAY_ON              SSD1306_DISPLAY | 0b1111
#define SSD1306_DISPLAYALLON_RESUME 0xA4

//** addressing
#define SSD1306_ADDRESSING              0b00100000 // 0x20
#define SSD1306_ADDRESSING_HORIZONTAL   0b00
#define SSD1306_ADDRESSING_VERTICAL     0b01
#define SSD1306_ADDRESSING_PAGE         0b10
#define SSD1306_SET_COLUMN              0b00100001 // 0x21
#define SSD1306_SET_PAGE                0b00100010 // 0x22

//** hardware configuration
#define SSD1306_SET_START_LINE          0b01000000 // 0x40
#define SSD1306_START_LINE_DEFAULT      0b00000000
#define SSD1306_SET_SEG_SCAN            0b10100000 // 0xA0
#define SSD1306_SET_SEG_SCAN_DEFAULT    SSD1306_SET_SEG_SCAN | 0b00
#define SSD1306_SET_SEG_SCAN_REVERSE    SSD1306_SET_SEG_SCAN | 0b01
#define SSD1306_SET_MULTIPLEX_RATIO     0b10101000 // 0xA8
#define SSD1306_MULTIPLEX_RATIO_DEFAULT 0b00111111
#define SSD1306_SET_COM_SCAN            0b11000000 // 0xC0
#define SSD1306_SET_COM_SCAN_DEFAULT    SSD1306_SET_COM_SCAN | 0b0000
#define SSD1306_SET_COM_SCAN_REVERSE    SSD1306_SET_COM_SCAN | 0b1000
#define SSD1306_SET_DISPLAY_OFFSET      0b11010011 // 0xD3
#define SSD1306_DISPLAY_OFFSET_DEFAULT  0b00000000
#define SSD1306_SET_COM_PINS            0b11011010 // 0xDA
#define SSD1306_COM_PINS_DEFAULT        0b00010010

//** timing and driving
#define SSD1306_SET_CLOCK_FREQUENCY     0b11010101 // 0xD5
#define SSD1306_CLOCK_FREQUENCY_DEFAULT 0b10000000
#define SSD1306_SET_PRECHARGE           0b11011001 // 0xD9
#define SSD1306_PRECHARGE_DEFAULT       0b00100010
#define SSD1306_SET_VCOMH_DESELECT      0b11011011 // 0xDB
#define SSD1306_VCOMH_DESELECT_DEFAULT  0b00100000
#define SSD1306_SET_CHARGE_PUMP         0b10001101 // 0x8D
#define SSD1306_CHARGE_PUMP_ENABLE      0b00010100
#define SSD1306_NOP 0xE3


void oledWriteCmd(byte command)
{
  digitalWrite(PIN_DC, LOW);
  //digitalWrite(PIN_CS, LOW); //our CS is tied to GND all the time!
  shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, command);
  //digitalWrite(PIN_CS, HIGH);
}

void SSD1306_OledInit()
{
  //restore power to the 
  pinMode(PowerPin, OUTPUT);
  digitalWrite(PowerPin, HIGH);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_SDIN, OUTPUT);
  pinMode(PIN_SCLK, OUTPUT);
  delay(5); //at least 5ms delay must be here

  oledWriteCmd(SSD1306_SET_MULTIPLEX_RATIO);  //0b10101000 = 0xA8
  oledWriteCmd(SSD1306_MULTIPLEX_RATIO_DEFAULT); //0b00111111  =0x3F 
  //0x3F  Multiplex Ratio for 128x64 (64-1) //this only affects position on screen! - different for different screen aspect ratios 

  oledWriteCmd(SSD1306_DISPLAY_RESET); //0b10100100 =0xA4 =Normal display mode

  oledWriteCmd(SSD1306_SET_DISPLAY_OFFSET); //0b11010011 // 0xD3 
  oledWriteCmd(SSD1306_DISPLAY_OFFSET_DEFAULT); //0b00000000

  oledWriteCmd(SSD1306_SET_START_LINE | SSD1306_START_LINE_DEFAULT);//0b01000000 =0x40

  // setting seg and com scan reverse means our x,y origin is top left of the screen
  oledWriteCmd(SSD1306_SET_SEG_SCAN_REVERSE);// 0b10100001 =0xA1 //note 0b10100000 =default
  oledWriteCmd(SSD1306_SET_COM_SCAN_REVERSE);// 0b11001000 =0xc8 //default = 0b11000000 = 0xC0

  oledWriteCmd(SSD1306_SET_COM_PINS);  //0b11011010 = 0xDA
  oledWriteCmd(SSD1306_COM_PINS_DEFAULT); //0b00010010  =0x12

  oledWriteCmd(SSD1306_SET_CONTRAST); //0b10000001 = 0x81  (2-byte)
  //oledWriteCmd(SSD1306_CONTRAST_DEFAULT); //0b01111111 = 0x7F  
  //0x7F default draws about 20% less current than full contrast and the screen is dimmer
  oledWriteCmd(0xFF); // but I like high contrast!

  oledWriteCmd(0xa4); //disable entire display on
  oledWriteCmd(0xa6); //set normal display

  oledWriteCmd(SSD1306_SET_CLOCK_FREQUENCY);  //0b11010101 = 0xD5  //FOSC is the oscillator frequency.
  //oledWriteCmd(SSD1306_CLOCK_FREQUENCY_DEFAULT); //0b10000000 default
  oledWriteCmd(0b11110010); //(4Bit: Fosc) (4Bit: Clkdiv) //lowering clkdiv reduces current a bit
  //freq is left-most 4 bits 0000=slowest ~300khz,  1000 = default = middle ~400khz, 1111=fastest ~550khz oscilator freq
  //frequencies https://community.arduboy.com/t/unsolvable-vsync-issues-and-screen-tearing/140/20
  
  oledWriteCmd(SSD1306_SET_CHARGE_PUMP); //0b10001101 = 0x8D
  oledWriteCmd(SSD1306_CHARGE_PUMP_ENABLE); //0b00010100 = 0x14 =use internal VCC  //0x10=external vcc

/*
 if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x9F); }
    else 
      { ssd1306_command(0xCF); }
*/

  oledWriteCmd(SSD1306_SET_PRECHARGE);  //0b11011001 = 0xD9
  oledWriteCmd(SSD1306_PRECHARGE_DEFAULT); //0b00100010 =0x22 //default for internal vcc // 0xF1 for EXTERNALVCC

  oledWriteCmd(SSD1306_SET_VCOMH_DESELECT); //0b11011011 =0xDB  //This command adjusts the VCOMH regulator output.
  oledWriteCmd(SSD1306_VCOMH_DESELECT_DEFAULT); //0b00100000 =0x20 default  // or 0x40?
  //oledWriteCmd(0x40);  // https://github.com/GravCorp/DayOLED_SSD1306/blob/master/DayOLED_SSD1306.cpp

  oledWriteCmd(SSD1306_DISPLAY_ON); //0b10101111 =0xAF 
  oledWriteCmd(SSD1306_DISPLAY_RESET); //0b10100100 =0xA4 =Normal display mode

//  for scrolling: (not used here)
//  oledWriteCmd(0x27); //set right horizontal scroll
//  oledWriteCmd(0x0);  //dummy byte
//  oledWriteCmd(0x0);  //page start address
//  oledWriteCmd(0x7);  //scroll speed
//  oledWriteCmd(0x7);  //page end address
//  oledWriteCmd(0x0);  //dummy byte
//  oledWriteCmd(0xff); //dummy byte
//  oledWriteCmd(0x2f); //start scrolling

}

void oledClear() {
  oledWriteCmd(SSD1306_DISPLAY_SLEEP);            //= 0b10101110 =0xAE disable display output while erasing the memory
  oledWriteCmd(SSD1306_ADDRESSING);               //0b00100000 // 0x20
  oledWriteCmd(SSD1306_ADDRESSING_HORIZONTAL);    //0b00
  //OR oledWriteCmd(SSD1306_ADDRESSING_VERTICAL);    //SSD1306_ADDRESSING_VERTICAL = 0b01 
  for(int i=0; i<1024; i++) oledWriteData(0x00);  //segement count =1024 =total # pix on screen)
  oledWriteCmd(SSD1306_ADDRESSING);               //0b00100000 // 0x20
  oledWriteCmd(SSD1306_ADDRESSING_PAGE);          //0b10  = 0x02
  oledWriteCmd(SSD1306_DISPLAY_ON);               //0b10101111 =0xAF 
  oledWriteCmd(SSD1306_DISPLAY_RESET);            //0b10100100 =0xA4 =back to Normal display mode
  }

void oledSetXY(int x, int y) {
  oledWriteCmd(SSD1306_SET_COLUMN);          //set column start & end address //0b00100001 //0x21  
  oledWriteCmd(x * SSD1306_FONT_WIDTH);      //set column start address // x*5
  oledWriteCmd(SSD1306_PIXEL_WIDTH - 1);     //set column end address // 128-1 =127 
  oledWriteCmd(SSD1306_SET_PAGE);            //set row start & end //0b00100010 //0x22
  oledWriteCmd(y);                           //row start
  oledWriteCmd(SSD1306_PAGE_HEIGHT - 1);     //row end //SSD1306_PAGE_HEIGHT=SSD1306_PIXEL_HEIGHT / SSD1306_PAGE_COUNT =7
}

//cascade: oledWriteString-->oledWriteCharacter-->oledWriteData
void oledWriteString(char *characters)
{while (*characters) oledWriteCharacter(*characters++);}

void oledWriteCharacter(char character)
{ 
  for (int i=0; i<5; i++) {
    oledWriteData(EEPROM.read(((character - 0x2D)*5)+i)); //starting at (-) character
    } 
  oledWriteData(0x00);
}

void oledWriteData(byte data)
{
  digitalWrite(PIN_DC, HIGH);
  //digitalWrite(PIN_CS, LOW); //not needed - our CS pin is tied to GND!
  shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
  //digitalWrite(PIN_CS, HIGH);
}

//the slit fonts cascade: 
//oledSetXY(x,y)
//oledWriteBigNumberTops->oledWriteBigCharacterTops->oledWriteData
//followed by oledSetXY(x,y+1)
//then
//oledWriteBigNumberBottoms->oledWriteBigCharacterBottoms->oledWriteData

void oledWriteBigNumberTops(char *characters)
{while(*characters) oledWriteBigCharacterTops(*characters++);}
void oledWriteBigCharacterTops(char character)
{
  //byte bytebuffer;
  for(int i=0; i<11; i++){
    if((character - 0x2d)>=0){
    oledWriteData(EEPROM.read(235+i+((character - 0x2d)*11)));
    //the big number font "Tops" start at eeprom memory address 235
    }
  }
  oledWriteData(0x00);//one row of spacer pixels
  oledWriteData(0x00);
  oledWriteData(0x00);
  oledWriteData(0x00);
}

void oledWriteBigNumberBottoms(char *characters)
{while(*characters) oledWriteBigCharacterBottoms(*characters++);}
void oledWriteBigCharacterBottoms(char character)
{ 
  for(int i=0; i<11; i++){
    if((character - 0x2d)>=0){
    oledWriteData(EEPROM.read(378+((character - 0x2d)*11)+i)); //starting at (-) character
    //the big number font "Bottoms" start at eeprom memory address 378
    }
  }
  oledWriteData(0x00);//one row of spacer pixels
  oledWriteData(0x00);
  oledWriteData(0x00);
  oledWriteData(0x00);
}

void setup()
{
  SSD1306_OledInit();
  oledClear();
}

void loop()
{
  SSD1306_OledInit();
  oledClear();
  
  oledSetXY(0, 0);
  oledWriteString("LOOP.TIME");//in microseconds
  lastMicros = thisMicros;thisMicros = micros();
  
  oledSetXY(0, 2);
  oledWriteBigNumberTops(dtostrf(thisMicros - lastMicros,7,0,tmp)); 
  // Note with dtostrf the #digits should match actual, or you get strange behavior from dtostrf
  oledSetXY(0, 3); 
  oledWriteBigNumberBottoms(dtostrf(thisMicros - lastMicros,7,0,tmp));
  
  delay(5000);

  //to turn off the display:
  oledWriteCmd(SSD1306_DISPLAY_SLEEP); //= 0b10101110 =0xAE  brings current down to 10uA
  delay(1);
  PORTC = PORTC & 0b11110000; //Set pins A0-A3 low smultaneously & leaves others untouched.
  //which is equivalent to:
  //digitalWrite(PowerPin, LOW); //if pin powering - bringing all lines low
  //digitalWrite(PIN_DC, LOW); 
  //digitalWrite(PIN_SDIN, LOW); 
  //digitalWrite(PIN_SCLK, LOW); 
  
  delay(2000);
}


/*  font defs - just for reference:
static const byte ASCII[][5] PROGMEM =
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
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
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
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
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
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f →
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
*/
