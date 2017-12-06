//PCD8544 controller  datasheet   84x48 pixel black and white LCDs
//https://www.sparkfun.com/datasheets/LCD/Monochrome/Nokia5110.pdf
//26x28 pixels are a nice size for graphics
//http://ianlangelectronic.webeden.co.uk/lcd-module-0/4569058582
//Also see: Sparkfuns Graphic LCD Hookup Guide for graphic bitmaps
//https://learn.sparkfun.com/tutorials/graphic-lcd-hookup-guide?_ga=2.193476516.863067337.1512481661-1112697269.1408048121 

#define RST 5          //6.1.12 RES: LOW followed by HIGH = reset
#define ChipEnable 6   //6.1.10 SCE: chip enable active LOW //could just connect this to ground if you have dedicated wires
#define DCmodeSelect 7 //6.1.9 D/C: mode select // If D/C LOW, the current byte is interpreted as command byte  If D/C = HIGH, the following bytes are stored in the display data RAM. 
#define DataIN 8       //6.1.7 SDIN: serial data line
#define SerialCLK 9    //6.1.8 SCLK: serial clock line  0.0 to 4.0 Mbits/s max, -line rests low, so could be used for light control

#define POWERpin  17  //this version powers the 5110 screen from pin A3
/*
Using analogue pins for digital I/O is just the same as using digital ones.
A0 is referred to as Pin 14
A1 is referred to as Pin 15
A2 is referred to as Pin 16
A3 is referred to as Pin 17
A4 is referred to as Pin 18
A5 is referred to as Pin 19
 */

//To make these fonts: LCD creator / LCD assistant http://www.instructables.com/id/Nokia-5110-graphics-tutorial/
/*
 * start with a bitmap that is 11 pixels wide, by 16 pixels high
 * for this 'split letters' method, use the vertical & little endian encoding in LCD assistant
 * the put the first 11 bytes in the Big11x16numberTops & the second 11 bytes for each number
 * in the Big11x16numberBottoms array
*/

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

//the regular sized font set:
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

void LcdWriteBigStringTops(char *characters)
{while(*characters) LcdWriteBigCharacterTops(*characters++);}

void LcdWriteBigStringBottoms(char *characters)
{while(*characters) LcdWriteBigCharacterBottoms(*characters++);}

void LcdWriteBigCharacterTops(char character)
{
  for(int i=0; i<11; i++){
    if((character - 0x2d)>=0){
  LcdWriteData(pgm_read_byte(&Big11x16numberTops[character - 0x2d][i]));
    }
  }
  LcdWriteData(0x00);//one row of spacer pixels inserted between characters
}

void LcdWriteBigCharacterBottoms(char character)
{ 
  for(int i=0; i<11; i++){
    if((character - 0x2d)>=0){
  LcdWriteData(pgm_read_byte(&Big11x16numberBottoms[character - 0x2d][i]));
    }
  }
  LcdWriteData(0x00);//one row of spacer pixels inserted between characters
}


void LcdWriteString(char *characters)
{
  while(*characters) LcdWriteCharacter(*characters++);
}

void LcdWriteCharacter(char character)
{
  for(int i=0; i<5; i++){
  LcdWriteData(pgm_read_byte(&ASCII[character - 0x20][i]));
  }
  LcdWriteData(0x00);//one row of spacer pixels inserted between characters
}

void LcdWriteData(byte dat)
{
  digitalWrite(DCmodeSelect, HIGH); //DCmodeSelect pin is low for commands
  digitalWrite(ChipEnable, LOW);
  shiftOut(DataIN, SerialCLK, MSBFIRST, dat); //transmit serial data
  digitalWrite(ChipEnable, HIGH);
}

void LcdXY(int x, int y)
{
  LcdWriteCmd(0x80 | x);  // Column.
  LcdWriteCmd(0x40 | y);  // Row.
}

void LcdWriteCmd(byte cmd)
{
  digitalWrite(DCmodeSelect, LOW); //DCmodeSelect pin is low for commands
  digitalWrite(ChipEnable, LOW);
  shiftOut(DataIN, SerialCLK, MSBFIRST, cmd); //transmit serial data
  digitalWrite(ChipEnable, HIGH);
}

void LcdInit(void)
{
  digitalWrite(RST, LOW);
  digitalWrite(RST, HIGH);
  LcdWriteCmd(0x21);  // LCD extended commands
  LcdWriteCmd(0xB8);  // set LCD Vop (contrast) //you may need to tweak this 
  LcdWriteCmd(0x04);  // set temp coefficent
  LcdWriteCmd(0x14);  // LCD bias mode 1:40  //you may need to tweak this
  LcdWriteCmd(0x20);  // LCD basic commands
  LcdWriteCmd(0x0C);  // LCD normal video
  
  for(int i=0; i<504; i++) LcdWriteData(0x00); // clear LCD
}

void setup()
{
  pinMode(POWERpin, OUTPUT);
  digitalWrite(POWERpin, HIGH);
  pinMode(RST, OUTPUT);
  pinMode(ChipEnable, OUTPUT);
  pinMode(DCmodeSelect, OUTPUT);
  pinMode(DataIN, OUTPUT);
  pinMode(SerialCLK, OUTPUT);
  
  LcdInit();

  LcdXY(0,2);
  LcdWriteString("VOLTAGE:"); 
}

char string[8];

void loop()
{
  digitalWrite(POWERpin, HIGH); //I've replaced the LED limiters with 3k3 resistors
  //so the whole 5110 display can be powered by a single digital pin @~1mA
  LcdInit();
  LcdXY(0,0); //position the cursor
  LcdWriteString("VOLTAGE:"); 
  
  //put the output from a voltage divider on AO
  float voltage = analogRead(A0) * 3.3 / 1024; // I'm using a 3.3v promini
  LcdXY (0,1);
  LcdWriteString(dtostrf(voltage,5,2,string)); //this is the numbers in the regular font
  
  //now print the output with double sized numbers
  //this takes two passes - the first prints the top of the numbers
  //and the second pass prints the bottom of each of the numbers
  // note only 7 digits fit across the display, and this is a numbers only font
  LcdXY (0,2);  //top half of the double-size numbers
  LcdWriteBigStringTops(dtostrf(voltage,5,2,string));
  LcdXY (0,3);  //bottom half of the numbers
  LcdWriteBigStringBottoms(dtostrf(voltage,5,2,string));

  delay(5000);
  digitalWrite(POWERpin, LOW);
  delay(5000); 
  
}

