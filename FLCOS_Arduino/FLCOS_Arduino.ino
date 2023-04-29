//This example shows how to animate graphics on a VGA screen. No backbuffering is used... just try it.
//You need to connect a VGA screen cable and an external DAC (simple R2R does the job) to the pins specified below.
//cc by-sa 4.0 license
//bitluni

#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include <math.h>

#include <SPI.h>
#include <Wire.h>

//pin configuration
const int redPins[] = {2, 4, 12, 13};
const int greenPins[] = {15, 16, 17, 18};
const int bluePins[] = {21, 22, 23};
const int hsyncPin = 32;
const int vsyncPin = 33;
const int clockPin = 14;
//19
//27

//VGA Device
VGA11Bit vga;

const int tvp_addr = 0x5C;  // Shown as "0xB8" in the datasheet, due to the bit shift thing (8-bit I2C address vs 7-bit).

const int fidPin = 34;
const int intPin = 35;

const int pclkPin = 32;
const int hdPin = 33;
const int vdPin = 25;
const int validPin = 26;
const int d0Pin = 27;
const int d1Pin = 14;
const int d2Pin = 12;
const int d3Pin = 13;
const int d4Pin = 22;
const int d5Pin = 21;
const int d6Pin = 17;
const int d7Pin = 16;

const int sclPin = 15;
const int sdaPin = 2;
const int rstnPin = 0;

const int spimosiPin = 23;
const int spimisoPin = 19;
const int spisclkPin = 18;

const int sdscnPin = 5;
const int flscnPin = 4;

char my_string[100];

//vspi
SPIClass * vspi = NULL;
static const int spiFreq = 30000;  // 30 KHz

void flWrite(byte addr, byte data) {
  vspi->beginTransaction(SPISettings(spiFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(flscnPin, LOW);    // Pull csn low to prep other end for transfer
  vspi->transfer(addr&0x7f);      // Send the reg address. (MSB of the address stays Low for a WRITE).
  vspi->transfer(data);           // Write the data to the reg.
  vspi->endTransaction();
  digitalWrite(flscnPin, HIGH);   // Set csn high to signify end of data transfer
}

char spiRead(byte addr) {
  vspi->beginTransaction(SPISettings(spiFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(flscnPin, LOW);          // Pull csn low to prep other end for transfer
  vspi->transfer( 0x80 | (addr&0x7f) ); // Set the MSB high for a READ.
  char spi_read = vspi->transfer(0x00); // Send dummy 0x00 byte, so we can read the reg back.
  vspi->endTransaction();
  digitalWrite(flscnPin, HIGH);         // Set csn high to signify end of data transfer
  return spi_read;
}

char tvp_read(char reg_addr) {
  Wire.beginTransmission(tvp_addr);
  Wire.write(reg_addr);             // Reg address to READ from.
  Wire.endTransmission();
  Wire.requestFrom(tvp_addr,1);     // Request ONE byte.
  char i2c_read = Wire.read();      // Read the byte from the buffer.
  return i2c_read;
};

char tvp_write(char reg_addr, char data) {
  Wire.beginTransmission(tvp_addr);
  Wire.write(reg_addr);             // Reg address to WRITE to.
  Wire.write(data);
  Wire.endTransmission();
};

void tvp_init() {
  uint8_t tvpreg_00 = 0x00;  // [7:2]=RES=b000000. [1]=0=AIP1A Comp, 1=A1P1B Comp. [0]=0=Comp, 1=S-Vid (bit 1 ignorred in S-Vid mode).
  uint8_t tvpreg_01 = 0x15;  // [7:5]=RES=b000. [4]=1. [3:2]=AGC Offset=b01. [1:0]=AGC Gain=b01.
  uint8_t tvpreg_02 = 0x00;  // [7:3]=RES=b00000. [2]=PLL Freeze=0. [1]=RES=0? [0]=PowerDown=0.
  uint8_t tvpreg_03 = 0x8F;  // [7]=(INT pin)=1=VBLK. [6]=0=GPCL outputs 0. [5]=0=GPCL is input. [4]=HVLK=0. [3]=YUV (TVPOE) Ena=1. [2]H/VSYNC, AVID, FID Ena=1. [1]=VBLK=0. [0]=SCLK Ena=1.

  uint8_t tvpreg_0d = 0x00;  // [7]=RES=0. [6]=YUV code range=0=ccir601. [5]=UV Code Format=0=Offset. [4:3]=YUV Bypass=b00=Normal. [2:0]=b000=ccir601 style, b111=ccir656 with embedded syncs.
  uint8_t tvpreg_0e = 0x00;  // [7:2]=RES=b000000. [1:0]=Luma trap=b00=No Notch.
  uint8_t tvpreg_0f = 0x02;  // [7]=RES. [0]=0=SCLK (YUV clock), 1=PCLK?.

  uint8_t tvpreg_c2 = 0x04;  // YUV (VDPOE) Ena=1.
  
  tvp_write(0x00, tvpreg_00);
  tvp_write(0x01, tvpreg_01);
  tvp_write(0x02, tvpreg_02);
  
  tvp_write(0x03, tvpreg_03);
  
  tvp_write(0x0d, tvpreg_0d);
  
  tvp_write(0x0e, tvpreg_0e);
  tvp_write(0x0f, tvpreg_0f);

  tvp_write(0xc2, tvpreg_c2);

  Serial.println();
  sprintf(my_string, "TVP Reg 0x00: 0x%02X. ", tvp_read(0x00)); Serial.print(my_string);
  if ( tvpreg_00&1 ) Serial.println("S-Video. AIP1A=Luma. AIP1B=Chroma.");
  else { Serial.print("Composite: "); if ( !(tvpreg_00&2) ) Serial.println("AIP1A Input."); else Serial.println("AIP1B Input."); }
  
  sprintf(my_string, "TVP Reg 0x01: 0x%02X. AGC Offset 0x%02X. AGC Gain 0x%02X.\n", tvp_read(0x01), (tvpreg_01&0xC)>>2, (tvpreg_01&3) ); Serial.print(my_string);
  
  sprintf(my_string, "TVP Reg 0x02: 0x%02X. PLL Freeze: %d. PowerDown: %d\n", tvp_read(0x02), (tvpreg_02&4), (tvpreg_02&1) ); Serial.print(my_string);
  
  sprintf(my_string, "TVP Reg 0x03: 0x%02X. VBKO: %d. GPCL: %d. GPCL_IO: %d. HVLK: %d. YUV/TVPOE: %d. HVSYNC/AVID/FID: %d. VBLK: %d. SCLK: %d.\n",
    tvp_read(0x03), (tvpreg_03&0x80)>>7, (tvpreg_03&0x40)>>6, (tvpreg_03&0x20)>>5, (tvpreg_03&0x10)>>4, (tvpreg_03&0x08)>>3, (tvpreg_03&0x04)>>2, (tvpreg_03&0x02)>>1, tvpreg_03&0x01 ); Serial.print(my_string);
  
  sprintf(my_string, "TVP Reg 0x04: 0x%02X. NTSC/PALN/PALM Autoswitch normal. \n", tvp_read(0x04)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x05: 0x%02X. [0]=Software Reset.\n", tvp_read(0x05)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x06: 0x%02X. Color Killer=Auto. Threshold=-24dB.\n", tvp_read(0x06)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x07: 0x%02X. Luma Processing #1.\n", tvp_read(0x07)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x08: 0x%02X. Luma Processing #2.\n", tvp_read(0x08)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x09: 0x%02X. Brightness Control.\n", tvp_read(0x09)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x0a: 0x%02X. Saturation Control.\n", tvp_read(0x0a)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x0b: 0x%02X. Hue Control.\n", tvp_read(0x0b)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x0c: 0x%02X. Contrast Control.\n", tvp_read(0x0c)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x0d: 0x%02X. Outputs and Data Rates.\n", tvp_read(0x0d)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x0e: 0x%02X. Luma Processing #3.\n", tvp_read(0x0e)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x0f: 0x%02X. Configure Shared Pins.\n", tvp_read(0x0f)); Serial.print(my_string);

  sprintf(my_string, "TVP Reg 0x84: 0x%02X. Vertical Line Count MSB.\n", tvp_read(0x84)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x85: 0x%02X. Vertical Line Count LSB.\n", tvp_read(0x85)); Serial.print(my_string);

  sprintf(my_string, "TVP Reg 0x86: 0x%02X. Interrupt Status Register B.\n", tvp_read(0x86)); Serial.print(my_string);
  sprintf(my_string, "TVP Reg 0x88: 0x%02X. Status Register #1.\n", tvp_read(0x88)); Serial.print(my_string);

  sprintf(my_string, "TVP Reg 0xc2: 0x%02X. Interrupt Configuration Reg A\n", tvp_read(0xc2)); Serial.print(my_string);
}

void flcos_init() {
  /*
  // JAKKS Projector power-up settings...
  flWrite(0x01, 0x15);  // [7]=RES=0. [6:5]=b00="Use Inputs for video timing"? (JAKKS).  [4:3]=DitherMode. [2:0]=DataMode b000=RGB, b101=ccir656/601.
  flWrite(0x06, 0x48);  // [7]=RES=0. [6]=VFlip=1. [5]=HFlip=0. [4]=RES=0. [3]=NSleepMode=1 SET this bit to wake up!. [2:0]=RES=b000.
  flWrite(0x0e, 0x00);  // [7:4]=RES=b0000. [3:0]=GammaValue=9=Gamma of 1.0.
  */

  flWrite(0x00, 0x01);  // MUST be set to 0x01.

  flWrite(0x01, 0x15);  // [7]=RES=0. [6:5]=b00="Use Inputs for video timing"? (JAKKS).  [4:3]=DitherMode. [2:0]=DataMode b000=RGB, b101=ccir656/601.
  
  flWrite(0x02, 0x00);  // [7:5]=VPol,HPol,ValidPol=b000=Active High,High,High. [4]=Arctic Mode?? [3:2]=RES=b00. [1:0]=VideoMask=b00 for 8-bit.
  
  flWrite(0x03, 0xFF);  // [7:6]=RES, always set to b11. [5:0]=LED Brightness=Max (0x3F).
  
  flWrite(0x04, 0x08);  // [7:4]=VldDelOffset. [3:2]=VertInterpMode. [1:0]=HVldDelay[9:8].
  flWrite(0x05, 0x00);  // [7:0]=HVldDelay[7:0].
  
  flWrite(0x06, 0x48);  // [7]=RES=0. [6]=VFlip=1. [5]=HFlip=0. [4]=RES=0. [3]=NSleepMode=1 SET this bit to wake up!. [2:0]=RES=b000.

  flWrite(0x07, 0x04);  // [7:5]=RES=b000. [4]=HScaleCycle[8]. [3:2]=VScaleStep[9:8]=b01. [1:0]=HScaleStep[9:8].
  flWrite(0x08, 0xD6);  // [7:0]=HScaleStep[7:0].
  flWrite(0x09, 0x05);  // [7:0]=HScaleCycle[7:0].
  flWrite(0x0a, 0xDE);  // [7:0]=VScaleStep[7:0].
  flWrite(0x0b, 0x0E);  // [7:0]=VScaleCycle[7:0].

  flWrite(0x0e, 0x00);  // [7:4]=RES=b0000. [3:0]=GammaValue=0=Gamma of 1.0.  // JAKKS.
  //flWrite(0x0e, 0x09);  // [7:4]=RES=b0000. [3:0]=GammaValue=9=Gamma of 2.1.    // Guessed.
  
  // 288 lines per field PAL...
  uint16_t hscalestep = 214;  // H Coef.
  uint16_t hscalecycle = 5;   // H Cycle.
  uint16_t vscalestep = 399;  // V Coef.
  uint16_t vscalecycle = 7;   // V Cycle.
  
  uint8_t flreg07 = ((hscalecycle&0x100)>>4) | ((vscalestep&0x300)>>6) | ((hscalestep&0x300)>>8);
  uint8_t flreg08 = hscalestep  & 0xff;
  uint8_t flreg09 = hscalecycle & 0xff;
  uint8_t flreg0a = vscalestep  & 0xff;
  uint8_t flreg0b = vscalecycle & 0xff;
  flWrite(0x07, flreg07);  // [7:5]=RES=b000. [4]=HScaleCycle[8]. [3:2]=VScaleStep[9:8]=b01. [1:0]=HScaleStep[9:8].
  flWrite(0x08, flreg08);  // [7:0]=HScaleStep[7:0].
  flWrite(0x09, flreg09);  // [7:0]=HScaleCycle[7:0].
  flWrite(0x0a, flreg0a);  // [7:0]=VScaleStep[7:0].
  flWrite(0x0b, flreg0b);  // [7:0]=VScaleCycle[7:0].
  
  flWrite(0x0c, 0x01);  // [7:0]=VVldDelay[7:0].
  flWrite(0x0d, 0x00);  // [7:4]=RES=b0000. [3:0]=CinemaLines=b0000.
  flWrite(0x0e, 0x09);  // [7:4]=RES=b0000. [3:0]=GammaValue=9=Gamma of 2.1.

  
  // Color Space Gain registers...
  /*
  flWrite(0x0f, 0x00);  // [7:0]=
  flWrite(0x10, 0x00);  // [7:0]=
  flWrite(0x11, 0x00);  // [7:0]=
  flWrite(0x12, 0x00);  // [7:0]=
  flWrite(0x13, 0x00);  // [7:0]=
  flWrite(0x14, 0x00);  // [7:0]=
  flWrite(0x15, 0x00);  // [7:0]=
  flWrite(0x16, 0x00);  // [7:0]=
  flWrite(0x17, 0x00);  // [7:0]=

  flWrite(0x18, 0x54);  // [7:4]=Resolution=5. [3:0]=Revision=4.

  // Color Space Offset registers...
  flWrite(0x19, 0x00);  // [7:0]=
  flWrite(0x1a, 0x00);  // [7:0]=
  flWrite(0x1b, 0x00);  // [7:0]=

  flWrite(0x1c, 0x2f);  // [7]=RES=0. [6:0]=MinVFreq[6:0]=0x2F.
  flWrite(0x1d, 0x3f);  // [7]=RES=0. [6:0]=MaxVFreq[6:0]=0x3F.
  */
  delay(50);
  
  Serial.println();
  sprintf(my_string, "FLCOS Reg 0x00: 0x%02X\n", spiRead(0x00)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x01: 0x%02X\n", spiRead(0x01)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x02: 0x%02X\n", spiRead(0x02)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x03: 0x%02X\n", spiRead(0x03)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x04: 0x%02X\n", spiRead(0x04)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x05: 0x%02X\n", spiRead(0x05)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x06: 0x%02X\n", spiRead(0x06)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x07: 0x%02X\n", spiRead(0x07)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x08: 0x%02X\n", spiRead(0x08)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x09: 0x%02X\n", spiRead(0x09)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x0a: 0x%02X\n", spiRead(0x0a)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x0b: 0x%02X\n", spiRead(0x0b)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x0c: 0x%02X\n", spiRead(0x0c)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x0d: 0x%02X\n", spiRead(0x0d)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x0e: 0x%02X\n", spiRead(0x0e)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x0f: 0x%02X\n", spiRead(0x0f)); Serial.print(my_string);

  /*
  sprintf(my_string, "FLCOS Reg 0x10: 0x%02X\n", spiRead(0x10)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x11: 0x%02X\n", spiRead(0x11)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x12: 0x%02X\n", spiRead(0x12)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x13: 0x%02X\n", spiRead(0x13)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x14: 0x%02X\n", spiRead(0x14)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x15: 0x%02X\n", spiRead(0x15)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x16: 0x%02X\n", spiRead(0x16)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x17: 0x%02X\n", spiRead(0x17)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x18: 0x%02X\n", spiRead(0x18)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x19: 0x%02X\n", spiRead(0x19)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x1a: 0x%02X\n", spiRead(0x1a)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x1b: 0x%02X\n", spiRead(0x1b)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x1c: 0x%02X\n", spiRead(0x1c)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x1d: 0x%02X\n", spiRead(0x1d)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x1e: 0x%02X\n", spiRead(0x1e)); Serial.print(my_string);
  sprintf(my_string, "FLCOS Reg 0x1f: 0x%02X\n", spiRead(0x1f)); Serial.print(my_string);
  */
  Serial.println();
}

//initial setup
void setup()
{
  pinMode(fidPin, INPUT);
  pinMode(intPin, INPUT);

  pinMode(pclkPin, INPUT);
  pinMode(hdPin, INPUT);
  pinMode(vdPin, INPUT);
  pinMode(validPin, INPUT);
  pinMode(d0Pin, INPUT);
  pinMode(d1Pin, INPUT);
  pinMode(d2Pin, INPUT);
  pinMode(d3Pin, INPUT);
  pinMode(d4Pin, INPUT);
  pinMode(d5Pin, INPUT);
  pinMode(d6Pin, INPUT);
  pinMode(d7Pin, INPUT);

  pinMode(sclPin, INPUT);
  pinMode(sdaPin, INPUT);

  pinMode(spimosiPin, OUTPUT);
  pinMode(spimisoPin, INPUT_PULLUP);
  pinMode(spisclkPin, OUTPUT);

  pinMode(sdscnPin, OUTPUT);
  digitalWrite(sdscnPin, HIGH);
  
  pinMode(flscnPin, OUTPUT);
  digitalWrite(flscnPin, HIGH);

  Serial.begin(115200);

  Wire.begin (sdaPin, sclPin);

  vspi = new SPIClass(VSPI);
  vspi->begin(spisclkPin, spimisoPin, spimosiPin, flscnPin);

  pinMode(rstnPin, OUTPUT);
  digitalWrite(rstnPin, LOW);   // Reset FLCOS and TVP for 100ms.
  delay(100);
  digitalWrite(rstnPin, HIGH);  // Bring FLCOS and TVP out of reset.
  delay(100);

  flcos_init();
  delay(100);
  tvp_init();

  uint8_t stat1;
  uint8_t stat1_old = 0xde;  // Random byte, to trigger the first printf. (TVP Status Reg often starts at 0x00, so we need any different value in stat1_old).

  uint8_t stat5;
  uint8_t stat5_old = 0xde;  // Random byte, to trigger the first printf. (TVP Status Reg often starts at 0x00, so we need any different value in stat5_old).

  while (1) {
    stat1 = tvp_read(0x88);
    if ( stat1 != stat1_old ) {
      sprintf(my_string, "TVP Reg 0x88: 0x%02X. Status Register #1. Peak: %d. Alt: %d. 50Hz: %d. Lost: %d. Color: %d. Vsync: %d. Hsync: %d. VCR: %d.\n",
        stat1, (stat1&0x80)>>7, (stat1&0x40)>>6, (stat1&0x20)>>5, (stat1&0x10)>>4, (stat1&0x08)>>3, (stat1&0x04)>>2, (stat1&0x02)>>1, (stat1&0x01) ); Serial.print(my_string); 
    }
    stat1_old = stat1;

    stat5 = tvp_read(0x8c);
    if ( stat5 != stat5_old ) {
      sprintf(my_string, "TVP Reg 0x8c: 0x%02X. Status Register #5. Autoswitch: %d. ", stat5, (stat5&0x80)>>7 ); Serial.print(my_string);
      switch ( stat5&0xF ) {
        case 1: sprintf(my_string, "(M) NTSC ITU−R BT.601           \n"); break;
        case 3: sprintf(my_string, "(B, G, H, I, N) PAL ITU−R BT.601\n"); break;
        case 5: sprintf(my_string, "(M) PAL ITU−R BT.601            \n"); break;
        case 7: sprintf(my_string, "(Combination-N) ITU−R BT.601    \n"); break;
        case 9: sprintf(my_string, "NTSC 4.43 ITU−R BT.601          \n"); break;
        default: sprintf(my_string, "Reserved.\n"); break;
      }
      Serial.print(my_string);
    }
    stat5_old = stat5;

    delay(500);
  }

  /*
  uint16_t hvid_start = 0;
  uint16_t hvid_stop = 0;

  uint8_t vvid_start = 0;
  uint8_t vvid_stop = 0;
  while(1) {
    tvp_write(0x11, hvid_start>>2);   // Write MSB bits [9:8] first. (have to shift RIGHT, to get MSB bit 9 into reg bit 7).
    tvp_write(0x12, hvid_start&3);    // Write LSB bits [7:0] to update the value.
    
    tvp_write(0x13, hvid_stop>>2);   // Write MSB bits [9:8] first. (have to shift RIGHT, to get MSB bit 9 into reg bit 7).
    tvp_write(0x14, hvid_stop&3);    // Write LSB bits [7:0] to update the value.
    
    sprintf(my_string, "hvid_start: %03d. hvid_stop: %03d. vvid_start: %02d. vvid_stop: %02d\n", hvid_start, hvid_stop, vvid_start, vvid_stop); Serial.print(my_string);
    delay(100);
    hvid_start++;
    if (hvid_start==511) {
      hvid_start = 0;
      vvid_start++;
    }
  }
  */

  /*
  while (1) {
    sprintf(my_string, "TVP Reg 0x88: 0x%02X. Status Register #1.\n", tvp_read(0x88)); Serial.print(my_string);
    delay(1000);
  }
  */
 
  /*
  //initializing i2s vga (with only one framebuffer)
  vga.init(vga.MODE200x150, redPins, greenPins, bluePins, hsyncPin, vsyncPin, clockPin);
  //setting the font
  vga.setFont(Font6x8);
  */
}

//the loop is done every frame
void loop()
{
  /*
  //setting the text cursor to the lower left corner of the screen
  vga.setCursor(0, vga.yres - 8);
  //setting the text color to white with opaque black background
  vga.setTextColor(vga.RGB(0xffffff), vga.RGBA(0, 0, 0, 255));
  //printing the fps
  vga.print("fps: ");
  static long f = 0;
  vga.print(long((f++ * 1000) / millis()));

  //circle parameters
  float factors[][2] = {{1, 1.1f}, {0.9f, 1.02f}, {1.1, 0.8}};
  int colors[] = {vga.RGB(0xff0000), vga.RGB(0x00ff00), vga.RGB(0x0000ff)};
  //animate them with milliseconds
  float p = millis() * 0.002f;
  for (int i = 0; i < 3; i++)
  {
    //calculate the position
    int x = vga.xres / 2 + sin(p * factors[i][0]) * vga.xres / 3;
    int y = vga.yres / 2 + cos(p * factors[i][1]) * vga.yres / 3;
    //clear the center with a black filled circle
    vga.fillCircle(x, y, 8, 0);
    //draw the circle with the color from the array
    vga.circle(x, y, 10, colors[i]);
  }
  //render the flame effect
  for (int y = 0; y < vga.yres - 9; y++)
    for (int x = 1; x < vga.xres - 1; x++)
    {
      //take the avarage from the surrounding pixels below
      int c0 = vga.get(x, y);
      int c1 = vga.get(x, y + 1);
      int c2 = vga.get(x - 1, y + 1);
      int c3 = vga.get(x + 1, y + 1);
      int r = ((c0 & 0x1f) + (c1 & 0x1f) + ((c2 & 0x1f) + (c3 & 0x1f)) / 2) / 3;
      int g = (((c0 & 0x3e0) + (c1 & 0x3e0) + ((c2 & 0x3e0) + (c3 & 0x3e0)) / 2) / 3) & 0x3e0;
      int b = (((c0 & 0x3c00) + (c1 & 0x3c00) + ((c2 & 0x3c00) + (c3 & 0x3c00)) / 2) / 3) & 0x3c00;
      //draw the new pixel
      vga.dotFast(x, y, r | g | b);
    }
   */
}
