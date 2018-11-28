/*
 * wrapper.cpp
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

#include <thread>
#include "threadobj.h"
#include "ht16k33.h"
#include "starburst.h"
#include "pstream.h"
#include "wrapper.h"

Wrapper::Wrapper(STARBURST *a)
{
    //start the stream
    proc = new redi::pstream("./polyglot", redi::pstream::pstdout |
                             redi::pstream::pstdin | redi::pstream::pstderr);
    hwDisplay = a;
}

//send to stream
void Wrapper::putStream(std::string a)
{
    *proc << a << std::endl;
}

//read from stream
std::string Wrapper::getStream(void)
{
    char buf[256];
    proc->getline(buf, sizeof(buf), '\n');
    std::string buffer = buf;
    //printf("STREAM RAW: %s\n", buffer.c_str()); //comment me out, ok!
    return buffer;
}

//clear stream
void Wrapper::clearStream(void)
{
    proc->clear();
}

void Wrapper::readEngine(void)
{
    while(1)
    {
        if(engineOutput.is() == 0) //if buffer is empty
        {
            std::string readBuffer = getStream(); //read the stream
            printf("RAW ENGINEIN: %s\n", readBuffer.c_str());
            if(!readBuffer.empty()) //if stream is full
            {
                int a = checkState(readBuffer); //check the state, no trim
                if(a) //if the state is valid
                {
                    printf("OPCODE: %d, DEBUG: %s\n", a, readBuffer.c_str()); //comment me out
                    engineOutput.set_str_easy(readBuffer); //set the output
                }
            }
        }
        cpuBreak();
    }
}

//send data to the display (cout emulation) + hardware display
void Wrapper::writeDispay(void)
{
    while(1)
    {
        if(displayInput.is() == 1) //if the buffer is full
        {
            std::string disp = displayInput.get_str(); //get the display data
            printf("STARBURST: %s\n", disp.c_str()); //cout emu, display the data

            hwDisplay->print(disp); //writes to hardware display, nothing fancy
        }
        cpuBreak();
    }
}

//send "stuff" to the engine
void Wrapper::writeEngine(void)
{
    while(1)
    {
        if(engineInput.is() == 1) //if the buffer is full
        {
            std::string input = engineInput.get_str(); //get the input data
            printf("RAW ENGINEOUT: %s\n", input.c_str()); //comment me out
            putStream(input); //write the input data to the engine
        }
        cpuBreak(); //HANDBREAK!!!!!
    }
}

//read button input (cin emulation)
void Wrapper::readButton(void)
{
    while(1)
    {
        if(buttonOutput.is() == 0) //if no button is available
        {
            //quick and dirty emulation for keypad
            //replace with real code later
            char a = getchar();
            if(a) //check input
            {
                int k = -1;
                if(a == '1') k=0; //RE
                if(a == '2') k=1; //CB
                if(a == '3') k=2; //CL
                if(a == '4') k=3; //EN

                if(a == 'q') k=4; //LV
                if(a == 'w') k=5; //DM
                if(a == 'e') k=6; //PB
                if(a == 'r') k=7; //PV

                if(a == 'a') k=8; //A1
                if(a == 's') k=9; //B2
                if(a == 'd') k=10; //C3
                if(a == 'f') k=11; //D4

                if(a == 'z') k=12; //E5
                if(a == 'x') k=13; //F6
                if(a == 'c') k=14; //G7
                if(a == 'v') k=15; //H8

                if(k!= -1) //if input is ok
                    buttonOutput.set_int_easy(k); //set button
            }
        }
        cpuBreak();
    }
}

int Wrapper::checkState(std::string &a, bool b) //found a better way!, b=1 for trim
{
    if(a.empty())
        return 0;

    size_t found;
    int return_val = 0;

    do
    {
        found = a.find(response[return_val]); //iterate through valid engine response
        return_val++;
        if(found!=std::string::npos)
            break;
    }
    while(return_val < 16);

    if(b) //trim string
    {
        if((return_val >= 12) && (return_val <=14))
        {
            if(return_val == 12)
                found+=5;
            else if(return_val == 13)
                found+=6;
            else
                found+=14;

            std::string b;
            for(std::string::size_type i = found; i < a.length(); i++)
                b+=a.at(i);
            a=b;
        }
    }
    return return_val%16;
}