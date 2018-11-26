/*
 * ht16k33.h - used to talk to the htk1633 chip to do things like turn on LEDs or scan keys
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

#ifndef ht16k33_h
#define ht16k33_h

class HT16K33
{
public:
    //vars
    uint16_t keyRam[3] = {0};
    uint16_t memory[8] = {0};
    //funk
    HT16K33();
    int begin(int address);
    int clrAll(void); //writes 128 0's to the chip
    void clrAllM(void); //writes 128 0's
    int setAll(void); //writes 128 1's to the chip
    void setAllM(void); //writes 128 1's
    int sleep(void); //puts the beast in standby mode
    int normal(void); //wake up and start oscillator
    int displayOn(void); //take a guess
    int displayOff(void); //turns the display off
    int setBrightness(int val); //0-15, 15 is max
    //add button read code
    uint8_t readINTflag(void); //read the interrupt flag
    //got the funk back
    int setBlinkRate(int rate); //sets the blink rate, 0-3, off -> fast!
    int setLedM(int index); //set led in buffer
    int setLed(int index); //set and show
    int update(void); //show buffer
    int clrLedM(int index); //clear led in buffer
    int clrLed(int index); //clear and show
    //i2c write
    int i2c_write(uint8_t data);
    int i2c_write(uint8_t addrMem, uint8_t data);
    int i2c_write_16(uint8_t addrMem, uint16_t data);
    int i2c_write_16(uint8_t addrMem, uint16_t *data, int size);
    //i2c read
    int i2c_read(uint8_t addrMem, uint8_t &data);
    int i2c_read(uint8_t addrMem, uint16_t *data, int size);
    //debug stuff
    int memDump(void); //for debugging 0x00-0x0E
    int keyDump(void); //for debugging 0x40-0x44, I might need to fix the bit ordering
private:
    int _address;
    int file_i2c;
};

#endif
