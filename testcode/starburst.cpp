/*
 * starburst.cpp - used to talk to the htk1633 chip to do things like turn on LEDs or scan keys
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

#include <stdint.h>
#include <stdio.h>
#include <unistd.h> //delay
#include <string>
#include "ht16k33.h"
#include "starburst.h"
#include "starburstfont.h"

// Constructor
STARBURST::STARBURST()
{
    //hi, I'm a comment, how are you
}

//init
int STARBURST::begin(int address, int digits)
{
    _digits = digits;
    return HT16K33::begin(address);
}

//delay function
void STARBURST::delay(int ms)
{
    if(ms == 0)
        return;
    usleep( ms * 1000 );
}

//clear buffer
void STARBURST::clrAllM(void)
{
    for(int i = 0; i < _digits; i++)
        memory[i] = 0x0000;
}

//clears only the used segments
int STARBURST::clrAll(void)
{
    clrAllM(); //could be trimmed
    return i2c_write_16(0x00, memory, _digits);
}

//set buffer
void STARBURST::setAllM(void)
{
    for(int i = 0; i < _digits; i++)
        memory[i] = 0xFFFF;
}

//sets only the used segments
int STARBURST::setAll(void)
{
    setAllM();
    return i2c_write_16(0x00, memory, _digits);
}

//displays char @ digit with dp
int STARBURST::print(char a, int digit, bool dp)
{
    if(printM(a, digit, dp))
        return 1;
    return i2c_write_16(addrMap[digit], memory[digit]);
}

//sends to buffer
int STARBURST::printM(char a, int digit, bool dp)
{
    if((a >= 0) && (a < 128))
    {
        uint16_t i = alphafonttable[(uint8_t)a];
        if(dp)
            i |= 0x4000; //enable dp
        memory[digit] = i;
        return 0;
    }
    return 1;
}

//sends all to display
int STARBURST::update(void)
{
    return i2c_write_16(0x00, memory, _digits); //page write
}

// shifts buffer to the left, requires update to see
void STARBURST::shiftML(void)
{
    for(int i=0; i < _digits-1; i++)
        memory[i] = memory[i+1];
    memory[_digits-1] = 0x0000;
}

// shifts buffer to the right, requres update to see
void STARBURST::shiftMR(void)
{
    for(int i = _digits-1; i > 0; i--)
        memory[i] = memory[i-1];
    memory[0] = 0x0000;
}

//prints string
int STARBURST::print(std::string a)
{
    int j = 0;
    j|= clrAll();
    for(unsigned int i=0; i < a.length(); i++)
    {
        shiftML();
        j|= printM(a[i], _digits-1);
    }
    j|= update();
    return j;
}

//prints string with timed scroll
int STARBURST::print(std::string a, int ms)
{
    int j = 0;
    j|= clrAll();
    for(unsigned int i=0; i < a.length(); i++)
    {
        shiftML();
        j|= printM(a[i], _digits-1);
        j|= update();
        delay(ms);
    }
    for(int i=0; i < _digits; i++)
    {
        shiftML();
        j|= update();
        delay(ms);
    }
    return j;
}
