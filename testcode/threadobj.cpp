/*
 * threadobj.cpp
 *
 * 	.▄▄ · ▄▄▌   ▄▄▄· .▄▄ ·  ▄ .▄▄▄▄▄·  ▄· ▄▌▄▄▄▄▄▄▄▄ .
 *  ▐█ ▀. ██•  ▐█ ▀█ ▐█ ▀. ██▪▐█▐█ ▀█▪▐█▪██▌•██  ▀▄.▀·
 *  ▄▀▀▀█▄██▪  ▄█▀▀█ ▄▀▀▀█▄██▀▐█▐█▀▀█▄▐█▌▐█▪ ▐█.▪▐▀▀▪▄
 *  ▐█▄▪▐█▐█▌▐▌▐█ ▪▐▌▐█▄▪▐███▌▐▀██▄▪▐█ ▐█▀·. ▐█▌·▐█▄▄▌
 *   ▀▀▀▀ .▀▀▀  ▀  ▀  ▀▀▀▀ ▀▀▀ ··▀▀▀▀   ▀ •  ▀▀▀  ▀▀▀
 *                                         11-28-2018
 *		Slash/Byte
 *
 */
#include <string>
#include "threadobj.h"

//returns true is there is something stored
bool ThreadBuffer::is(void)
{
    return _is;
}

//stores string, return true if already full
bool ThreadBuffer::set_str(std::string a)
{
    if(is()) return 1;
    strBuf = a;
    _is = 1;
    return 0;
}

//stores an int, returns true if already full
bool ThreadBuffer::set_int(int a)
{
    if(is()) return 1;
    intBuf = a;
    _is = 1;
    return 0;
}

//stores a string, waits till things are empty before it writes
void ThreadBuffer::set_str_easy(std::string a)
{
    bool _k;
    do
    {
        _k = set_str(a);
    }
    while(_k);
}

//stores a int, waits till things are empty before it writes
void ThreadBuffer::set_int_easy(int a)
{
    bool _k;
    do
    {
        _k = set_int(a);
    }
    while(_k);
}

//returns stored string
std::string ThreadBuffer::get_str(void)
{
    _is = 0;
    return strBuf;
}

//returns stored int
int ThreadBuffer::get_int(void)
{
    _is = 0;
    return intBuf;
}