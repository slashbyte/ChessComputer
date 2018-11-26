/*
 * ht16k33.cpp - used to talk to the htk1633 chip to do things like turn on LEDs or scan keys
 * Copyright:  Slash/Byte
 * License: GPLv3
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * History:
 * 2015-10-04  Peter Sjoberg <peters-alib AT techwiz.ca>
 *             Created using https://www.arduino.cc/en/Hacking/LibraryTutorial and ht16k33 datasheet
 * 2015-11-25  Peter Sjoberg <peters-alib AT techwiz DOT ca>
 *	       first check in to github
 * 2016-08-09  René Wennekes <rene.wennekes AT gmail.com>
 *             Contribution of 7-segment & 16-segment display support
 *             Added clearAll() function
 *
 * 2018-11-11  Slash/byte <I don't remember my email AT this time>
 *             Ported the code to the raspberry pi
 *             Broke the key scanning feature lol
 *             he also plans on fixing it later
 *
 * 	.▄▄ · ▄▄▌   ▄▄▄· .▄▄ ·  ▄ .▄▄▄▄▄·  ▄· ▄▌▄▄▄▄▄▄▄▄ .
 *  ▐█ ▀. ██•  ▐█ ▀█ ▐█ ▀. ██▪▐█▐█ ▀█▪▐█▪██▌•██  ▀▄.▀·
 *  ▄▀▀▀█▄██▪  ▄█▀▀█ ▄▀▀▀█▄██▀▐█▐█▀▀█▄▐█▌▐█▪ ▐█.▪▐▀▀▪▄
 *  ▐█▄▪▐█▐█▌▐▌▐█ ▪▐▌▐█▄▪▐███▌▐▀██▄▪▐█ ▐█▀·. ▐█▌·▐█▄▄▌
 *   ▀▀▀▀ .▀▀▀  ▀  ▀  ▀▀▀▀ ▀▀▀ ··▀▀▀▀   ▀ •  ▀▀▀  ▀▀▀
 *                                         11-11-2018
 */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>          //Needed for I2C port
#include <fcntl.h>           //Needed for I2C port
#include <sys/ioctl.h>       //Needed for I2C port
#include <linux/i2c-dev.h>   //Needed for I2C port
#include "ht16k33.h"

#define SETBIT(x,n) x |= (1 << n)
#define CLRBIT(x,n) x &= ~(1 << n)
//#define GET_RIGHTMOST_16_BITS(x) ((x) & (unsigned int)0x0000FFFF)
#define GET_LOW_8_BITS(x) ((x) & (uint16_t)0xFF)
#define GET_HIGH_8_BITS(x) (((x) & ~((uint16_t)0xFF)) >> 8)

// "address" is base address 0-7 which becomes 11100xxx = E0-E7
//#define BASEHTADDR 0x70

//Commands
#define HT16K33_DDAP          0x00 // B00000000 // Display data address pointer: 0000xxxx
#define HT16K33_SS            0x20 // B00100000 // System setup register
#define HT16K33_SS_STANDBY    0x00 // B00000000 // System setup - oscillator in standby mode
#define HT16K33_SS_NORMAL     0x01 // B00000001 // System setup - oscillator in normal mode
#define HT16K33_KDAP          0x40 // B01000000 // Key Address Data Pointer
#define HT16K33_IFAP          0x60 // B01100000 // Read status of INT flag
#define HT16K33_DSP           0x80 // B10000000 // Display setup
#define HT16K33_DSP_OFF       0x00 // B00000000 // Display setup - display off
#define HT16K33_DSP_ON        0x01 // B00000001 // Display setup - display on
#define HT16K33_DSP_NOBLINK   0x00 // B00000000 // Display setup - no blink
#define HT16K33_DSP_BLINK2HZ  0x02 // B00000010 // Display setup - 2hz blink
#define HT16K33_DSP_BLINK1HZ  0x04 // B00000100 // Display setup - 1hz blink
#define HT16K33_DSP_BLINK05HZ 0x06 // B00000110 // Display setup - 0.5hz blink
#define HT16K33_RIS           0xA0 // B10100000 // ROW/INT Set
#define HT16K33_RIS_OUT       0x00 // B00000000 // Set INT as row driver output
#define HT16K33_RIS_INTL      0x01 // B00000001 // Set INT as int active low
#define HT16K33_RIS_INTH      0x03 // B00000011 // Set INT as int active high
#define HT16K33_DIM           0xE0 // B11100000 // Dimming set
#define HT16K33_DIM_1         0x00 // B00000000 // Dimming set - 1/16
#define HT16K33_DIM_2         0x01 // B00000001 // Dimming set - 2/16
#define HT16K33_DIM_3         0x02 // B00000010 // Dimming set - 3/16
#define HT16K33_DIM_4         0x03 // B00000011 // Dimming set - 4/16
#define HT16K33_DIM_5         0x04 // B00000100 // Dimming set - 5/16
#define HT16K33_DIM_6         0x05 // B00000101 // Dimming set - 6/16
#define HT16K33_DIM_7         0x06 // B00000110 // Dimming set - 7/16
#define HT16K33_DIM_8         0x07 // B00000111 // Dimming set - 8/16
#define HT16K33_DIM_9         0x08 // B00001000 // Dimming set - 9/16
#define HT16K33_DIM_10        0x09 // B00001001 // Dimming set - 10/16
#define HT16K33_DIM_11        0x0A // B00001010 // Dimming set - 11/16
#define HT16K33_DIM_12        0x0B // B00001011 // Dimming set - 12/16
#define HT16K33_DIM_13        0x0C // B00001100 // Dimming set - 13/16
#define HT16K33_DIM_14        0x0D // B00001101 // Dimming set - 14/16
#define HT16K33_DIM_15        0x0E // B00001110 // Dimming set - 15/16
#define HT16K33_DIM_16        0x0F // B00001111 // Dimming set - 16/16

// Constructor
HT16K33::HT16K33()
{
    //COMMENTS!!!!
}

//init
int HT16K33::begin(int address)
{
    _address = address;
    // For older raspberry pi modules use
    // "/dev/i2c-0" instead of
    // "/dev/i2c-1" for the i2c port
    char *filename = (char*)"/dev/i2c-1";
    if ((file_i2c = open(filename, O_RDWR)) < 0)
    {
        //ERROR HANDLING: you can check errno to see what went wrong
        printf("Failed to open the i2c bus");
        return 1;
    }
	
    if (ioctl(file_i2c, I2C_SLAVE, _address) < 0)
    {
        //ERROR HANDLING; you can check errno to see what went wrong
        printf("Failed to acquire bus access and/or talk to slave.\n");
        return 1;
    }

    int i = 0;
    i|= i2c_write(HT16K33_SS  | HT16K33_SS_NORMAL); // Awaken the best
    i|= i2c_write(HT16K33_DSP | HT16K33_DSP_ON | HT16K33_DSP_NOBLINK); // Display on and no blinking
    i|= i2c_write(HT16K33_RIS | HT16K33_RIS_OUT); // INT pin works as row output
    i|= i2c_write(HT16K33_DIM | HT16K33_DIM_16);  // Brightness set to max
    return i; //return i2c write state
}

//write 8-bit byte
int HT16K33::i2c_write(uint8_t data)
{
    uint8_t a[1] = {data};
    if((write(file_i2c, a, 1))!=1)
    {
        printf("Failed to write to the i2c bus.\n");
        return 1;
    }
    return 0;
}

//write 16 bits, 8-bit address & 8-bit of data
int HT16K33::i2c_write(uint8_t addrMem, uint8_t data)
{
    uint8_t a[2] = {0};
    a[0] = addrMem;
    a[1] = data;
    if((write(file_i2c, a, 2))!=2)
    {
        printf("Failed to write to the i2c bus.\n");
        return 1;
    }
    return 0;
}

//write 24 bits, 8-bit address & 16-bit of data
int HT16K33::i2c_write_16(uint8_t addrMem, uint16_t data)
{
    uint8_t a[3] = {0};
    a[0] = addrMem;
    a[1] = GET_LOW_8_BITS(data);
    a[2] = GET_HIGH_8_BITS(data); //don't forget to bring a towel
    if((write(file_i2c, a, 3))!=3)
    {
        printf("Failed to write to the i2c bus.\n");
        return 1;
    }
    return 0;
}

//write nth bits, 8-bit address & page write?
int HT16K33::i2c_write_16(uint8_t addrMem, uint16_t *data, int size)
{
    int _size = (2*size)+1;
    uint8_t a[_size] = {0};
    a[0] = addrMem;
    int j = 0;
    for(int i = 1; i < _size; i++)
    {
        a[i] = GET_LOW_8_BITS(data[j]);
        a[++i] = GET_HIGH_8_BITS(data[j]);
        j++;
    }

    if((write(file_i2c, a, _size))!=_size)
    {
        printf("Failed to write to the i2c bus.\n");
        return 1;
    }
    return 0;
}

//reads 8bits @address
int HT16K33::i2c_read(uint8_t addrMem, uint8_t &data)
{
    uint8_t a[1] = {0};
    if(i2c_write(addrMem))
        return 1;
    int i = read(file_i2c, a, 1);
    if (i != 1)
    {
        //ERROR HANDLING: i2c transaction failed
        printf("Failed to read from the i2c bus.\n");
        return i;
    }
    data = a[0];
    return 0;
}

//page read of 16bits @address
int HT16K33::i2c_read(uint8_t addrMem, uint16_t *data, int size)
{
    int _size = size*2;
    uint8_t a[_size] = {0};
    if(i2c_write(addrMem))
        return 1;
    int i = read(file_i2c, a, _size);
    if (i != _size)
    {
        //ERROR HANDLING: i2c transaction failed
        printf("Failed to read from the i2c bus.\n");
        return i;
    }

    int j = 0;
    for(int i = 0; i < size; i++)
    {
        data[i] = (a[j+1] << 8) | a[j];
        j+=2;
    }
    return 0;
}

//reads and dumps the chip memory
int HT16K33::memDump(void)
{
    uint16_t a[8] = {0};
    int j = i2c_read(HT16K33_DDAP, a, 8);
    if(j!=0)
    {
        printf("Dump error %d\n", j);
        return 1;
    }
    printf("---| MEMDUMP |---\n");
    for(int i = 0; i < 8; i++)
        printf("0x%02X ..... 0x%04X\n", i*2, a[i]);
    return 0;
}

//reads and dumps the key press "ram"
int HT16K33::keyDump(void)
{
    uint16_t a[3] = {0};
    int j = i2c_read(0x40, a, 3);
    if(j!=0)
    {
        printf("Dump error %d\n", j);
        return 1;
    }
    printf("---| KEYDUMP |---\n");
    for(int i = 0; i < 3; i++)
        printf("0x%02X ..... 0x%04X\n", (i*2)+0x40, a[i]);
    return 0;
}

//clr buffer
void HT16K33::clrAllM(void)
{
    for(int i = 0; i < 8; i++)
        memory[i] = 0x0000;
}

//sets the chip mem to all zeros
int HT16K33::clrAll(void)
{
    clrAllM();
    return i2c_write_16(HT16K33_DDAP, memory, 8);
}

//set buffer
void HT16K33::setAllM(void)
{
    for(int i = 0; i < 8; i++)
        memory[i] = 0xFFFF;
}

//sets the chip mem to all ones
int HT16K33::setAll(void)
{
    setAllM();
    return i2c_write_16(HT16K33_DDAP, memory, 8);
}

//puts in standby mode
int HT16K33::sleep(void)
{
    return i2c_write(HT16K33_SS|HT16K33_SS_STANDBY);
}

//wake and start oscillator
int HT16K33::normal(void)
{
    return i2c_write(HT16K33_SS|HT16K33_SS_NORMAL);
}

//turns display on
int HT16K33::displayOn(void)
{
    return i2c_write(HT16K33_DSP |HT16K33_DSP_ON);
}

//turns display off
int HT16K33::displayOff(void)
{
    return i2c_write(HT16K33_DSP |HT16K33_DSP_OFF);
}

//sets the brightness of the display
int HT16K33::setBrightness(int val)
{
    if (HT16K33_DIM_1>=0 && val <HT16K33_DIM_16)
        return i2c_write(HT16K33_DIM|val);
    return 1;
}

//reads the interrupt flag
uint8_t HT16K33::readINTflag(void)
{
    uint8_t a = 0;
    int i = i2c_read(HT16K33_IFAP, a);
    if(i!=0)
        return 1;
    return a;
}

//sets the blink rate
//0-3, off -> fast!
int HT16K33::setBlinkRate(int rate)
{
    int _rate = 0;
    if((rate >= 0) && (rate <= 3))
    {
        if(rate == 1)
            _rate = HT16K33_DSP_BLINK05HZ;
        else if(rate == 2)
            _rate = HT16K33_DSP_BLINK1HZ;
        else if(rate == 3)
            _rate = HT16K33_DSP_BLINK2HZ;
        else
            _rate = HT16K33_DSP_NOBLINK;
        return i2c_write(HT16K33_DSP | _rate | HT16K33_DSP_ON);
    }
    return 1;
}

//set led in mem
int HT16K33::setLedM(int index)
{
    if((index >= 0) && (index < 128))
    {
        int _i = index/16;
        int _b = index%16;
        SETBIT(memory[_i],_b);
        return 0;
    }
    return 1;
}

//set in mem then show
int HT16K33::setLed(int index)
{
    if(setLedM(index))
        return 1;
    return update();
}

//sends all to display
int HT16K33::update(void)
{
    return i2c_write_16(HT16K33_DDAP, memory, 8); //page write
}

//clr led in mem
int HT16K33::clrLedM(int index)
{
    if((index >= 0) && (index < 128))
    {
        int _i = index/16;
        int _b = index%16;
        CLRBIT(memory[_i],_b);
        return 0;
    }
    return 1;
}

//clr in mem then show
int HT16K33::clrLed(int index)
{
    if(clrLedM(index))
        return 1;
    return update();
}