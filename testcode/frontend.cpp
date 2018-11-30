/*
 * frontend.cpp
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
#include <string>
#include "pstream.h"
#include "ht16k33.h"
#include "starburst.h"
#include "threadobj.h"
#include "wrapper.h"
#include "frontend.h"

//begins the threads
void FrontEnd::begin(void)
{
	//start thread
    t1 = std::thread(&FrontEnd::readEngine, this);
    t2 = std::thread(&FrontEnd::writeEngine, this);
    t3 = std::thread(&FrontEnd::readButton, this);
    t4 = std::thread(&FrontEnd::writeDisplay, this);
	//background
    t1.detach();
    t2.detach();
    t3.detach();
    t4.detach();
}

//returns true if a button is pressed
bool FrontEnd::is_button(void)
{
    return buttonOutput.is();
}

//returns the result of the button press
int FrontEnd::get_button(void)
{
    return buttonOutput.get_int();
}

//returns true is there is something from the engine
bool FrontEnd::is_engine(void)
{
    return engineOutput.is();
}

//gets the engine sting
std::string FrontEnd::get_engine(void)
{
    return engineOutput.get_str();
}

//send a string to the hardware display
void FrontEnd::send_display(std::string a)
{
    displayInput.set_str_easy(a);
}

//sends a string to the hardware display, b=1 for uppercase, b=0 for lower
void FrontEnd::send_display(std::string a, bool b)
{
    if(b)strToUpper(a);
    else strToLower(a);
    displayInput.set_str_easy(a);
}

//send a string to the engine
void FrontEnd::send_engine(std::string a)
{
    engineInput.set_str_easy(a);
}

//converts a string to uppercase
void FrontEnd::strToUpper(std::string &a)
{
    for(std::string::size_type i = 0; i < a.length(); i++)
        if((a.at(i) >= 97) && (a.at(i) <= 122)) a.at(i) = a.at(i)-32;
}

//converts a sting to lowercase
void FrontEnd::strToLower(std::string &a)
{
    for(std::string::size_type i = 0; i < a.length(); i++)
        if((a.at(i) >= 65) && (a.at(i) <= 90)) a.at(i) = a.at(i)+32;
}

//converts a button code to a char
char FrontEnd::buttonToChar(int a, int b) //yeah I could have used math...
{
    if(b%2)
		return num[a-8];
    else 
		return alfa[a-8];
}
//////////////////////////////////////////////////////
//undo, the computers last move, then your last move
void FrontEnd::undoMove(void)
{
	while(displayBusy()); //hwDisplay lock check
	saveDisplayState();
    send_engine("remove"); //I think thats undo
	send_display("Undo");
	hwDisplay->delay(500); //wait before going back to the screen
	restoreDisplayState();
}

//invoke the engine
void FrontEnd::go(void)
{
    send_engine("go");
}

//start a new game
void FrontEnd::newGame(int sd, bool p) //new game
{
    std::string _sd = "sd " + std::to_string(sd); //convert search depth to string
    send_engine("new");
    send_engine(_sd);
	if(p)
		send_engine("hard");
	else
		send_engine("easy");
    send_display("New ");
}

//turns on ponder, think on opponents move, uses the cpu 100% of the time
void FrontEnd::ponder(bool &p)
{
	while(displayBusy()); //hwDisplay lock check
	saveDisplayState();
	p = !p;
	if(p)
	{
		send_engine("hard");
		send_display("On  ");
	}
	else
	{
		send_engine("easy");
		send_display("Off ");
	}
	hwDisplay->delay(500);
	restoreDisplayState();
}

//get a users move
bool FrontEnd::getMove(int b) //get move from user
{
    std::string move = "----";
    move.at(0) = buttonToChar(b, 0); //alfa[a-8]; //convert to indexed char
    int digit = 1;
    send_display(move, 1); //update with first button press, upper
    while(1) //input loop
    {
        bool _b = is_button(); //while buttons !clear or !enter, grab user input
        if(_b)
        {
            int _c = get_button();
            if((_c >= 8) && (_c <= 15) && (digit < 4)) //buttons 8-15
            {
                move.at(digit) = buttonToChar(_c, digit);
                digit++;
                send_display(move, 1); //send to display, update, upper
            }
            if(_c == 2) //clear
            {
                send_display("----"); //send to display, update
                return 1;
            }
            if(_c == 3) //enter
            {
                send_engine(move); //send move to engine
                return 0;
            }
        }
        cpuBreak();
    }
    return 1;
}

//show the computers move
void FrontEnd::showMove(std::string b)
{
    send_display(b, 1); //send to display, upper
}

//show the computers hint
void FrontEnd::showHint(std::string b)
{
	while(displayBusy()); //hwDisplay lock check
	saveDisplayState();
    send_display(b); //send to display
	hwDisplay->delay(1000); //second delay for hint display
	restoreDisplayState();
}

//clear the display
void FrontEnd::clear(void) //clear display, fix later
{
    send_display("----"); //would blank be better?
}

//request position verification
void FrontEnd::posVer(void)
{
    send_engine("FEN"); //hacked polyglot command
}

//request current board
void FrontEnd::getBoard(void)
{
	send_engine("BOARD");
}

//used for debugging in the console
/////////////////////////////////////////////
void FrontEnd::printBoard(std::string a)
{
	std::string::size_type _l = a.length();
	if(_l == 17) //expected length
	{
		for(int i = 9; i < 17; i++) //magic numbers mean ice cream
			printf("%c ", a.at(i));
		printf("\n");
	}
	else
		printf("---------------\n");
}
//////////////////////////////////////////////

//ask for a hint
void FrontEnd::hint(void)
{
    send_engine("hint");
}

//display error screen
void FrontEnd::error(void)
{
	while(displayBusy()); //hwDisplay lock check
    send_display("____"); //clear display
    hwDisplay->setBlinkRate(3); //blink display
    bool _b;
    do
    {
        _b = is_button();
        cpuBreak();
    }
    while(!_b);  //wait for any-key to be pushed
    hwDisplay->setBlinkRate(0); //turn off blink
}

//check led control
void FrontEnd::check(bool b) //map to correct led on hardware
{
	while(displayBusy()); //hwDisplay lock check
    if(b)
		hwDisplay->setLed(14); //fix led num
    else
        hwDisplay->clrLed(14); //fix led num
}

//mate led control
void FrontEnd::mate(bool b) //map to correct led on hardware
{
	while(displayBusy()); //hwDisplay lock check
    if(b)
        hwDisplay->setLed(62); //fix led num
    else
        hwDisplay->clrLed(62); //fix led num
}

//flash when mate (the end of the game)
void FrontEnd::mateBlink(void)
{
	while(displayBusy()); //hwDisplay lock check
    hwDisplay->setBlinkRate(2); //blink display
    bool _b;
    do
    {
        _b = is_button();
        cpuBreak();
    }
    while(!_b);  //wait for any-key to be pushed
    hwDisplay->setBlinkRate(0); //turn off blink
}

//think screen animation
void FrontEnd::thinkANI(void)
{
	while(displayBusy()); //hwDisplay lock check
    bool _b;
    int i = 0;
    do
    {
        for(int j = 0; j < 4; j++)
            hwDisplay->printM(spinner.at(i),j); //put that shit in the display buffer
        hwDisplay->update(); //show on display
        i=(i+1)%4;
        hwDisplay->delay(100); //increase to ramming speed captain!, !!affects update time!!
		_b = is_engine() | is_button(); //break if engine or button
    }
    while(!_b);  //wait for any-key to be pushed
}

//generic think screen, no delay in move processing
void FrontEnd::think(void)
{
    send_display("****");
}

//if the engine gives up, display why
void FrontEnd::killScreen(int b, int _delay)
{
	while(displayBusy()); //hwDisplay lock check
	saveDisplayState();
    std::string temp = "    " + KS[b-5];
    int _tempL = temp.length();
    int i = 0;
    bool _b;
    do
    {
        for(int j = 0; j < 4; j++)
            hwDisplay->printM(temp.at((i+j)%_tempL), j); // writes to "display buffer"
        hwDisplay->update(); //pushes to display
        i=(i+1)%_tempL;
        hwDisplay->delay(_delay); //150ms delay, affects update time, might remove
        _b = is_button();
    }
    while(!_b);  //wait for any-key to be pushed
	restoreDisplayState();
}

//save the current display state
void FrontEnd::saveDisplayState(void)
{
	while(displayBusy()); //hwDisplay lock check
    for(int i = 0; i < 8; i++) //save display state
        mState[i] = hwDisplay->memory[i];
}

//restore the last display state
void FrontEnd::restoreDisplayState(void)
{
	while(displayBusy()); //hwDisplay lock check
    for(int i = 0; i < 8; i++)
        hwDisplay->memory[i] = mState[i]; //renew state
    hwDisplay->update(); //display saved state
}

//display the fen string on the hw display
void FrontEnd::fen(std::string b)
{
	while(displayBusy()); //hwDisplay lock check
    saveDisplayState();
    while(b.length()%4) //stretch to multiple of 4, yeah I don't like it either...
        b += " ";
    std::string::size_type i = 0;
    send_display("FEN-");
    while(1)
    {
        bool _b = is_button();
        if(_b)
        {
            int _c = get_button();
            if((_c == 3) || (_c == 2)) //enter or clear, return to play
            {
                restoreDisplayState();
                return;
            }
            if(_c == 7) //pv, advance by 4
            {
                if(i < b.length())
                {
                    for(int j = 0; j < 4; j++)
                        hwDisplay->printM(b.at(i+j),j);
                    hwDisplay->update();
                    i=i+4;
                }
                else
					send_display("-END");
            }
        }
        cpuBreak();
    }
}

//set the engine level
void FrontEnd::level(int &sd) //set level
{
    saveDisplayState();
    std::string _sd = "Lv";
    if(sd < 10)
        _sd += "0" + std::to_string(sd);
    else
        _sd += std::to_string(sd);
    send_display(_sd);
    while(1)
    {
        bool _b = is_button();
        if(_b)
        {
            int _c = get_button();
			if(_c == 2) //clear
			{
				restoreDisplayState();
                return;
			}
            if(_c == 3) //enter
            {
                send_engine("sd " + std::to_string(sd)); //set new level
                restoreDisplayState();
                return;
            }
            if(_c == 4) //lv button, increment
            {
                sd = 1+(sd%25);
                if(sd < 10)
                    _sd = "Lv0" + std::to_string(sd);
                else
                    _sd = "Lv" + std::to_string(sd);
                send_display(_sd);
            }
        }
        cpuBreak();
    }
}


