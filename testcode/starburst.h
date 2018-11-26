/*
 * starburst.h - used to talk to the htk1633 chip to do things like turn on LEDs or scan keys
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
 * 	.▄▄ · ▄▄▌   ▄▄▄· .▄▄ ·  ▄ .▄▄▄▄▄·  ▄· ▄▌▄▄▄▄▄▄▄▄ .
 *  ▐█ ▀. ██•  ▐█ ▀█ ▐█ ▀. ██▪▐█▐█ ▀█▪▐█▪██▌•██  ▀▄.▀·
 *  ▄▀▀▀█▄██▪  ▄█▀▀█ ▄▀▀▀█▄██▀▐█▐█▀▀█▄▐█▌▐█▪ ▐█.▪▐▀▀▪▄
 *  ▐█▄▪▐█▐█▌▐▌▐█ ▪▐▌▐█▄▪▐███▌▐▀██▄▪▐█ ▐█▀·. ▐█▌·▐█▄▄▌
 *   ▀▀▀▀ .▀▀▀  ▀  ▀  ▀▀▀▀ ▀▀▀ ··▀▀▀▀   ▀ •  ▀▀▀  ▀▀▀
 *                                         11-11-2018
 * Special function extension for use with
 * "Adafruit 0.54" Quad Alphanumeric FeatherWing Display - Red"
 *
 */

#ifndef starburst_h
#define starburst_h

class STARBURST : public HT16K33
{
public:
    //we got the funk
    STARBURST();
    int begin(int address, int digits);
    void delay(int ms); //millisecond delay
    int clrAll(void); //faster clear
    int setAll(void); //faster set
    void clrAllM(void); //smaller, use update
    void setAllM(void); //smaller, use update
    int print(char a, int digit, bool dp = 0); //displays char @ digit with dp
    int printM(char a, int digit, bool dp = 0); //not dead-pool, decimal point, use update
    int update(void); //update display from buffer, faster
    void shiftML(void); //requires update to see
    void shiftMR(void); //requires update to see
    int print(std::string a, int ms); //print string, with delay for scroll
    int print(std::string a); //clipped string print
private:
    uint8_t addrMap[8] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E};
    int _digits;
};

#endif