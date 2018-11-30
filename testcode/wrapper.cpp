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
	std::string buffer;
	if(!std::getline(*proc, buffer))
	{
		printf("Check your polyglot Install and .ini file\n");
		printf("also check your engine installation path\n");
		printf("Stream Error, see ya!\n");
		exit(EXIT_FAILURE); //don't do this at home kids
	}
    //printf("STREAM RAW: %s\n", buffer.c_str()); //comment me out, ok!
    return buffer;
}

//clear stream
void Wrapper::clearStream(void)
{
    proc->clear();
}

//reads the engine from the input
void Wrapper::readEngine(void)
{
    while(1)
    {
        if(engineOutput.is() == 0) //if buffer is empty
        {
            std::string readBuffer = getStream(); //read the stream
            //printf("RAW ENGINEIN: %s\n", readBuffer.c_str()); //debuging, comment me out
            if(!readBuffer.empty()) //if stream is full
            {
                int a = checkState(readBuffer); //check the state, no trim
                if(a) //if the state is valid
                {
                    //printf("OPCODE: %d, DEBUG: %s\n", a, readBuffer.c_str()); //comment me out
                    engineOutput.set_str_easy(readBuffer); //set the output
                }
            }
        }
        cpuBreak();
    }
}

//use this before you call the hwDisplay object from another function
//its best to wait and make sure writeDisplay isn't using it first
bool Wrapper::displayBusy(void)
{
	return _displayBusy;
}

//send data to the display (cout emulation) + hardware display	
void Wrapper::writeDisplay(void) //wow I really can't spell
{
    while(1)
    {
        if(displayInput.is() == 1) //if the buffer is full
        {
			_displayBusy = 1; //display lock
            std::string disp = displayInput.get_str(); //get the display data
            //printf("STARBURST: %s\n", disp.c_str()); //cout emu, display the data, debug
			
			//check & mate DP bug fix
			//have to use the displays DP for check and mate
			//till I get the boards populated
			//I hate writing software for hardware I don't have...
			//////////////////// Remove ////////////////////////////////////
			//preserve DP state
			bool _check = (hwDisplay->memory[0] & 0x4000); //Q&D fix
			bool _mate = (hwDisplay->memory[3] & 0x4000); //Q&D fix
			/////////////////////////////////////////////////////////
			if(!disp.empty())
				hwDisplay->print(disp); //writes to hardware display, nothing fancy
			////////////////// Remove ////////////////////////////////
			//re-enable DP state after write
			if(_check)
				hwDisplay->setLed(14); //Q&D fix
			if(_mate)
				hwDisplay->setLed(62); //Q&D fix
			/////////////////////////////////////////////////////////
			_displayBusy = 0; //display unlock
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
            //printf("RAW ENGINEOUT: %s\n", input.c_str()); //comment me out
			if(!input.empty())
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
			/////////////////////////////////////
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
			///////////////////////////////////////////////////////
        }
        cpuBreak();
    }
}

//checks the info passed in by polyglot
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