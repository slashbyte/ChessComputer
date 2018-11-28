/*
 * 	.▄▄ · ▄▄▌   ▄▄▄· .▄▄ ·  ▄ .▄▄▄▄▄·  ▄· ▄▌▄▄▄▄▄▄▄▄ .
 *  ▐█ ▀. ██•  ▐█ ▀█ ▐█ ▀. ██▪▐█▐█ ▀█▪▐█▪██▌•██  ▀▄.▀·
 *  ▄▀▀▀█▄██▪  ▄█▀▀█ ▄▀▀▀█▄██▀▐█▐█▀▀█▄▐█▌▐█▪ ▐█.▪▐▀▀▪▄
 *  ▐█▄▪▐█▐█▌▐▌▐█ ▪▐▌▐█▄▪▐███▌▐▀██▄▪▐█ ▐█▀·. ▐█▌·▐█▄▄▌
 *   ▀▀▀▀ .▀▀▀  ▀  ▀  ▀▀▀▀ ▀▀▀ ··▀▀▀▀   ▀ •  ▀▀▀  ▀▀▀
 *                                         11-27-2018
 *	demo code for chess challenger
 */

#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "pstream.h"

#include "ht16k33.h"
#include "starburst.h"

#define ADDR 0x70

#define HANDBREAK 5 //"when your cpu is out of control, handbrake" quotes by slash/byte
#define cpuBreak() (this_thread::sleep_for(chrono::milliseconds(HANDBREAK)))

using namespace std;

class ThreadBuffer
{
public:
    ThreadBuffer() {};
    //buffer access
    bool is(void)
    {
        return _is;
    };
    bool set_str(string a)
    {
        if(is()) return 1;
        strBuf = a;
        _is = 1;
        return 0;
    };
    bool set_int(int a)
    {
        if(is()) return 1;
        intBuf = a;
        _is = 1;
        return 0;
    };
    void set_str_easy(string a)
    {
        bool _k;
        do
        {
            _k = set_str(a);
        }
        while(_k);
    };
    void set_int_easy(int a)
    {
        bool _k;
        do
        {
            _k = set_int(a);
        }
        while(_k);
    };
    string get_str(void)
    {
        _is = 0;
        return strBuf;
    };
    int get_int(void)
    {
        _is = 0;
        return intBuf;
    };
private:
    //buffer
    bool _is = 0;
    string strBuf;
    int intBuf = 0;
};

class Wrapper //keeps all the candy fresh! shhh...
{
public:
    Wrapper(STARBURST *a);
    STARBURST* hwDisplay;
    //buffer object
    ThreadBuffer engineOutput; //from engine
    ThreadBuffer engineInput; //to engine
    ThreadBuffer displayInput; //to display
    ThreadBuffer buttonOutput; //from button
    //threads to do my bidding!
    void readEngine(void); //grab engine commands
    void writeDispay(void); //write to display
    void writeEngine(void); //write to engine
    void readButton(void); //read buttons
    //functions
    int checkState(string &a, bool b = 0); //get state code from string
	//string getState(int a){return response[a-1];};
private:
    redi::pstream* proc;
    const string response[15] =
    {
        "Error (unknown command):",
        "Illegal move:",
        "Black in check", //hacked polyglot
        "White in check", //hacked polyglot
        "1-0 {White mates}",
        "0-1 {Black mates}",
        "1/2-1/2 {Stalemate}",
        "1/2-1/2 {Draw by insufficient material}",
        "1/2-1/2 {Draw by fifty-move rule}",
        "1/2-1/2 {Draw by repetition}",
        "resign",
        "move",
        "Hint",
        "POLYGLOT FEN:", //hacked polyglot
        "POLYGLOT" //hacked polyglot
    };
    void putStream(string a);
    string getStream(void);
    void clearStream(void);
};

Wrapper::Wrapper(STARBURST *a)
{
    //start the stream
    proc = new redi::pstream("./polyglot", redi::pstream::pstdout |
                             redi::pstream::pstdin | redi::pstream::pstderr);
    hwDisplay = a;
}

//send to stream
void Wrapper::putStream(string a)
{
    *proc << a << endl;
}

//read from stream
string Wrapper::getStream(void)
{
    char buf[256];
    proc->getline(buf, sizeof(buf), '\n');
    string buffer = buf;
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
            string readBuffer = getStream(); //read the stream
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
            string disp = displayInput.get_str(); //get the display data
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
            string input = engineInput.get_str(); //get the input data
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

int Wrapper::checkState(string &a, bool b) //found a better way!, b=1 for trim
{
    if(a.empty())
        return 0;

    size_t found;
    int return_val = 0;

    do
    {
        found = a.find(response[return_val]); //iterate through valid engine response
        return_val++;
        if(found!=string::npos)
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

            string b;
            for(string::size_type i = found; i < a.length(); i++)
                b+=a.at(i);
            a=b;
        }
    }
    return return_val%16;
}

// front end class for the wrapper
class FrontEnd : public Wrapper
{
    using Wrapper::Wrapper;
public:
    void begin(void); //starts 9 million threads
    bool is_button(void)
    {
        return buttonOutput.is();
    };
    int get_button(void)
    {
        return buttonOutput.get_int();
    };
    bool is_engine(void)
    {
        return engineOutput.is();
    };
    string get_engine(void)
    {
        return engineOutput.get_str();
    };
    void send_display(string a)
    {
        displayInput.set_str_easy(a);
    };
    void send_display(string a, bool b)
    {
        if(b)strToUpper(a);
        else strToLower(a);
        displayInput.set_str_easy(a);
    };
    void send_engine(string a)
    {
        engineInput.set_str_easy(a);
    };
    void strToUpper(string &a)
    {
        for(string::size_type i = 0; i < a.length(); i++)
            if((a.at(i) >= 97) && (a.at(i) <= 122)) a.at(i) = a.at(i)-32;
    };
    void strToLower(string &a)
    {
        for(string::size_type i = 0; i < a.length(); i++)
            if((a.at(i) >= 65) && (a.at(i) <= 90)) a.at(i) = a.at(i)+32;
    };
    char buttonToChar(int a, int b) //yeah I could have used math...
    {
        if(b%2) return num[a-8];
        else return alfa[a-8];
    };
private:
    thread t1,t2,t3,t4;
    const char alfa[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    const char num[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
};

void FrontEnd::begin(void)
{
    t1 = thread(&FrontEnd::readEngine, this); //start thread
    t2 = thread(&FrontEnd::writeEngine, this);
    t3 = thread(&FrontEnd::readButton, this);
    t4 = thread(&FrontEnd::writeDispay, this);

    t1.detach(); //background
    t2.detach();
    t3.detach();
    t4.detach();
}

//undoes the computers move then your move,
//it then becomes your move again
void undoMove(FrontEnd &a) //undo move
{
    a.send_engine("remove"); //I think thats undo
}

//invoke the engine
void go(FrontEnd &a) //go Go GO!
{
    a.send_engine("go");
}

//start a new game
void newGame(FrontEnd &a, int sd) //new game
{
	string _sd = "sd " + to_string(sd); //convert search depth to string
    a.send_engine("new");
	a.send_engine(_sd);
	//a.send_engine("force"); //odd things happened....
    //a.send_engine("st 0"); //blitz mode for test, move to level or something
    a.send_display("New ");
}

//get a users move
bool getMove(FrontEnd &a, int b) //get move from user
{
    string move = "----";
    move.at(0) = a.buttonToChar(b, 0); //alfa[a-8]; //convert to indexed char
    int digit = 1;
    a.send_display(move, 1); //update with first button press, upper
    while(1) //input loop
    {
        bool _b = a.is_button(); //while buttons !clear or !enter, grab user input
        if(_b)
        {
            int _c = a.get_button();
            if((_c >= 8) && (_c <= 15) && (digit < 4)) //buttons 8-15
            {
                move.at(digit) = a.buttonToChar(_c, digit);
                digit++;
                a.send_display(move, 1); //send to display, update, upper
            }
            if(_c == 2) //clear
            {
                a.send_display("----"); //send to display, update
                return 1;
            }
            if(_c == 3) //enter
            {
                a.send_engine(move); //send move to engine
                return 0;
            }
        }
        cpuBreak();
    }
	return 1;
}

//show the computers move
void showMove(FrontEnd &a, string b)
{
    a.send_display(b, 1); //send to display, upper
}

//clear the display
void clear(FrontEnd &a) //clear display, fix later
{
    a.send_display("----"); //would blank be better?
}

//set the engine level
void level(FrontEnd &a, int &sd) //set level
{
	uint16_t mState[8];
	for(int i = 0; i < 8; i++) //save display state
		mState[i] = a.hwDisplay->memory[i];
	
	string _sd = "Lv";
	if(sd < 10)
		_sd += "0" + to_string(sd);
	else
		_sd += to_string(sd);
	a.send_display(_sd);
	while(1)
	{
        bool _b = a.is_button();
        if(_b)
        {
            int _c = a.get_button();
            if(_c == 3) //enter
			{
				a.send_engine("sd " + to_string(sd)); //set new level
				for(int i = 0; i < 8; i++)
					a.hwDisplay->memory[i] = mState[i]; //renew state
				a.hwDisplay->update(); //display saved state
                return;
			}
            if(_c == 4) //lv button
            {
				sd = 1+(sd%25);
				if(sd < 10)
					_sd = "Lv0" + to_string(sd);
				else
					_sd = "Lv" + to_string(sd);
				a.send_display(_sd);
            }
        }
        cpuBreak();
    }
}

//request position verification
void posVer(FrontEnd &a)
{
    a.send_engine("DUMP");
}

//ask for a hint
void hint(FrontEnd &a)
{
    a.send_engine("hint");
}

//display error screen
void error(FrontEnd &a)
{
	a.send_display("____"); //clear display
	a.hwDisplay->setBlinkRate(3); //blink display
	bool _b;
	do{
		_b = a.is_button();
		cpuBreak();
	}while(!_b); //wait for any-key to be pushed
	a.hwDisplay->setBlinkRate(0); //turn off blink
}

//check led control
void check(FrontEnd &a, bool b) //map to correct led on hardware
{
    if(b)
	{
        printf("check set\n"); //remove
		a.hwDisplay->setLed(14);
	}
    else
	{
        printf("check clear\n"); //remove
		a.hwDisplay->clrLed(14);
	}
}

//mate led control
void mate(FrontEnd &a, bool b) //map to correct led on hardware
{
    if(b)
	{
        printf("mate set\n"); //remove
		a.hwDisplay->setLed(62);
	}
    else
	{
        printf("mate clear\n"); //remove
		a.hwDisplay->clrLed(62);
	}
}

//flash when mate (the end of the game)
void mateBlink(FrontEnd &a)
{
	a.hwDisplay->setBlinkRate(2); //blink display
	bool _b;
	do{
		_b = a.is_button();
		cpuBreak();
	}while(!_b); //wait for any-key to be pushed
	a.hwDisplay->setBlinkRate(0); //turn off blink
}

//think screen animation
void thinkANI(FrontEnd &a)
{
	string spinner = "/-\\|";
	bool _b;
	int i = 0;
	do{
		for(int j = 0; j < 4; j++)
			a.hwDisplay->printM(spinner.at(i),j); //put that shit in the display buffer
		a.hwDisplay->update(); //show on display
		i=(i+1)%4;
		_b = a.is_engine() | a.is_button(); //break if engine or button
		a.hwDisplay->delay(100); //increase to ramming speed captain!, !!affects update!!
	}while(!_b); //wait for any-key to be pushed	
}

//generic think screen, no delay in move processing
void think(FrontEnd &a)
{
	a.send_display("****");
}

//if the engine gives up, display why
void killScreen(FrontEnd &a, int b, int _delay)
{
	string KS[] = {
		"White Mates", "Black Mates", "Stalemate", "Draw",
        "Draw by 50 rule", "Draw", "Resign"
	};
	string temp = "    " + KS[b-5];
	int _tempL = temp.length();
	int i = 0;
	bool _b;
	do{
		for(int j = 0; j < 4; j++)
			a.hwDisplay->printM(temp.at((i+j)%_tempL), j); // writes to "display buffer"
		a.hwDisplay->update(); //pushes to display
		i=(i+1)%_tempL;
		a.hwDisplay->delay(_delay); //150ms delay, affects update, might remove
		_b = a.is_button();
		//cpuBreak();
	}while(!_b); //wait for any-key to be pushed
}

//display the fen string on the hw display
void fen(FrontEnd &a, string b)
{
	uint16_t mState[8];
	for(int i = 0; i < 8; i++) //save display state
		mState[i] = a.hwDisplay->memory[i];
	
	printf("%s\n", b.c_str()); //for debug
	while(b.length()%4) //stretch to multiple of 4, yeah I don't like it either...
		b += " ";
	string::size_type i = 0;
	a.send_display("FEN-");
	while(1)
	{
        bool _b = a.is_button();
        if(_b)
        {
            int _c = a.get_button();
            if(_c == 3) //enter, return to play
			{
				for(int i = 0; i < 8; i++)
					a.hwDisplay->memory[i] = mState[i]; //renew state
				a.hwDisplay->update(); //display saved state
                return;
			}
            if(_c == 7) //pv, advance by 4
            {
				if(i < b.length())
				{
					for(int j = 0; j < 4; j++)
						a.hwDisplay->printM(b.at(i+j),j);
					a.hwDisplay->update();
					i=i+4;
				}
				else
					a.send_display("-END");
            }
        }
        cpuBreak();
    }
}

int main(void)
{
    printf("Chess Challenger\n");
    printf("Slash/Byte\n");
    printf("----------------------\n"); //mapped the buttons to the keyboard for testing on the PC
	printf("keyboard emulation map\n"); //still no hardware button code yet *sad*, slash/bytes lazy
	printf("1:RE, 2:CB, 3:CL, 4:EN\n");
	printf("Q:LV, W:DM, E:PB, R:PV\n");
	printf("A:A1, S:B2, D:C3, F:D4\n");
	printf("Z:E5, X:F6, C:G7, V:H8\n");
	printf("----------------------\n");

    STARBURST HT; //hardware display
    FrontEnd test(&HT); //polyglot + stockfish9 (or 8, 8's nice too... I guess)
	//thats right, I'm not smart enough to wright a "good" chess engine
	//but hey, that just means I can use any UCI compatible engine on my hardware!
	//assuming it works with polyglot (a UCI to win-board converter)
	//oh and polyglot supports opening books, so...
	//stockfish9 with an opening book, on hardware! *mind blown*
	//thanks science!

    HT.begin(ADDR, 4); //address of the display and the number of digits
    HT.clrAll();
    HT.print("Chess Challenger", 100);
    HT.print("----");
	
	//start the io threads, engine input/output & button/display IO, all independent
	test.begin(); //not sure if that was the best decision, but it works well
	
	//helper variables for game play logic
    bool _b;
    bool SIDE = 0;
    bool NEWGAME = 1;
    const bool WHITE = 0;
    const bool BLACK = 1;
	int searchDepth = 20;
	
    while(1) //a for loop that will run till the end of time!, or till the batteries die...
    {
        _b = test.is_button(); //check button
        if(_b) //if there was a button press
        {
            int _c = test.get_button(); //get button
            if(_c == 0) //RE, reset
			{
				NEWGAME = 1; //reset game flag
				SIDE = 0; //reset side to default
				mate(test, 0); //clear mate
                newGame(test, searchDepth); //new game
			}
            if(_c == 1); //CB, challenger bottom, blitz enable disable?
            if(_c == 2) //CL, clear
                clear(test); //clear the HW display
            if(_c == 3) //EN, enter
			{
				if(NEWGAME){SIDE=!SIDE;NEWGAME=0;}; //if the game is new, flop sides
                go(test); //send engine command
				//think(test); //think screen
				thinkANI(test); //think with animation
			}
            if(_c == 4) //LV, level
                level(test, searchDepth); //select your level of play
            if(_c == 5) //DM,"double move?" undo a move
				undoMove(test);
            if(_c == 6) //PB, problem mode
				hint(test);
            if(_c == 7) //PV, position verification
                posVer(test); //position verification request
            if((_c >= 8)&&(_c <= 15)) //alpha-num buttons
            {
				if(NEWGAME){NEWGAME=0;};
                if(!getMove(test, _c)) //if move was "entered"
				{
					check(test, 0); //clear check led
					//think(test); //show think screen
					thinkANI(test); //think with animation
				}
            }
        }
        _b = test.is_engine(); //check engine
        if(_b) //if theres a response from the engine
        {
            string _d = test.get_engine(); //get engine response
            int _a = test.checkState(_d, 1); //check and trim
            if((_a == 1) || (_a == 2)) //error or illegal move
                error(test); //error screen, flash @ 2hz
            if((_a == 3) && (SIDE == BLACK)) //black in check
                check(test, 1); //set check led
            if((_a == 4) && (SIDE == WHITE)) //white in check
                check(test, 1); //set check led
			if((_a == 5) && (SIDE == WHITE)) //white mates
				mate(test, 1); //set mate led
			if((_a == 6) && (SIDE == BLACK)) //black mates
				mate(test, 1); //set mate led
			if((_a == 5) || (_a == 6))
				mateBlink(test); //flash display @ 1hz, if mate (end of game)
			if((_a >= 7) && (_a <= 11)) //scroll kill-screen (draw message)
				killScreen(test, _a, 150); //@150ms/char speed
            if(_a == 12) //get computer move
                showMove(test, _d); //display the computers move
			if(_a == 13) //hint
				showMove(test, _d + "  "); //display computer hint, in different notation.
			if(_a == 14) //fen
				fen(test, _d); //display fen string
        }
        cpuBreak();
    }
    return 0;
}
