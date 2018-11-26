#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "pstream.h"

#include "ht16k33.h"
#include "starburst.h"

#define ADDR 0x70

#define HANDBREAK 5 //"when your cpu is out of control, handbrake" quotes by slash/byte

using namespace std;

class Wrapper //keeps all the candy fresh! shhh...
{
public:
    mutex m;
    Wrapper(STARBURST *a);
    STARBURST* hwDisplay;
    //threads to do my bidding!
    void readEngine(void); //grab engine commands
    void writeEngine(void); //write to engine
    void readButton(void); //read buttons
    void writeDispay(void); //write to display
    //from engine
    bool is_engineOutput(void); //checks for output
    bool set_engineOutput(string a); //sets to output to a buffer
    string get_engineOutput(void); //return output string
    //to engine
    bool is_engineInput(void); //soooooo tired of commenting things!
    bool set_engineInput(string a); //WHY!!!!!
    string get_engineInput(void);
    //to display
    bool is_display(void);
    bool set_display(string a);
    string get_display(void);
    //from button
    bool is_button(void);
    bool set_button(int a);
    int get_button(void);
    //funks
    int checkState(string &a, bool b = 0); //get state code from string
private:
    //stuff
    redi::pstream* proc;
    //stream funk
    void putStream(string a);
    string getStream(void);
    void clearStream(void);
    //buffers vars
    bool _engineOutput = 0;
    bool _engineInput = 0;
    string engineOutput;
    string engineInput;
    bool _button = 0;
    bool _display = 0;
    int button = 0;
    string display;
};

Wrapper::Wrapper(STARBURST *a)
{
    //start the stream
    proc = new redi::pstream("./polyglot", redi::pstream::pstdout |
                             redi::pstream::pstdin | redi::pstream::pstderr);
    hwDisplay = a; //use display class object, yay!
}

bool Wrapper::is_display(void) //is there display data?
{
    m.lock(); //do I even need to lock this?
    bool a = _display;
    m.unlock(); //is it even really doing anything!?
    return a; //is any of this even real!
}

bool Wrapper::set_display(string a) //set display data
{
    if(is_display())
        return 1;
    m.lock();
    display = a;
    _display = 1;
    m.unlock();
    return 0;
}

string Wrapper::get_display(void) //get display data
{
    m.lock();
    string a = display;
    _display = 0;
    m.unlock();
    return a;
}

bool Wrapper::is_button(void) //is the button a button? or a horse!
{
    m.lock();
    bool a = _button;
    m.unlock();
    return a;
}

bool Wrapper::set_button(int a) //set button
{
    if(is_button())
        return 1;
    m.lock();
    button = a;
    _button = 1;
    m.unlock();
    return 0;
}

int Wrapper::get_button(void) //get button
{
    m.lock();
    int a = button;
    _button = 0;
    m.unlock();
    return a;
}

bool Wrapper::is_engineOutput(void) //is the output there?
{
    m.lock();
    bool a = _engineOutput;
    m.unlock();
    return a;
}

bool Wrapper::set_engineOutput(string a) //set engine output
{
    if(is_engineOutput())
        return 1;
    m.lock();
    engineOutput = a;
    _engineOutput = 1;
    m.unlock();
    return 0;
}

string Wrapper::get_engineOutput(void) //get engine output
{
    m.lock();
    string a = engineOutput;
    _engineOutput = 0;
    m.unlock();
    return a;
}

bool Wrapper::is_engineInput(void) //is the input there?
{
    m.lock();
    bool a = _engineInput;
    m.unlock();
    return a;
}

bool Wrapper::set_engineInput(string a) //set engine input
{
    if(is_engineInput())
        return 1;
    m.lock();
    engineInput = a;
    _engineInput = 1;
    m.unlock();
    return 0;
}

string Wrapper::get_engineInput(void) //get engine input
{
    m.lock();
    string a = engineInput;
    _engineInput = 0;
    m.unlock();
    return a;
}

int Wrapper::checkState(string &a, bool b) //got to be a better way!, b=1 for trim
{
    if(a.empty())
        return 0;

    size_t found;

    found = a.find("Error (unknown command):");
    if(found!=string::npos)
        return 1;

    found = a.find("Illegal move:");
    if(found!=string::npos)
        return 2;

    found = a.find("Black in check"); //only in modd-ed polyglot
    if(found!=string::npos)
        return 3;

    found = a.find("White in check"); //only in modd-ed polyglot
    if(found!=string::npos)
        return 4;

    found = a.find("1-0 {White mates}");
    if(found!=string::npos)
        return 5;

    found = a.find("0-1 {Black mates}");
    if(found!=string::npos)
        return 6;

    found = a.find("1/2-1/2 {Stalemate}");
    if(found!=string::npos)
        return 7;

    found = a.find("1/2-1/2 {Draw by insufficient material}");
    if(found!=string::npos)
        return 8;

    found = a.find("1/2-1/2 {Draw by fifty-move rule}");
    if(found!=string::npos)
        return 9;

    found = a.find("1/2-1/2 {Draw by repetition}");
    if(found!=string::npos)
        return 10;

    found = a.find("resign");
    if(found!=string::npos)
        return 11;

    found = a.find("move");
    if(found!=string::npos)
    {
        if(!b) //no trim
        {
            found+=5;
            string b;
            for(string::size_type i = found; i < a.length(); i++)
                b+=a.at(i);
            a=b;
        }
        return 12;
    }

    found = a.find("Hint");
    if(found!=string::npos)
    {
        if(!b) //don't trim
        {
            found+=6;
            string b;
            for(string::size_type i = found; i < a.length(); i++)
                b+=a.at(i);
            a=b;
        }
        return 13;
    }

    found = a.find("POLYGLOT FEN:"); //only in modd-ed polyglot
    if(found!=string::npos)
    {
        if(!b) //don't trim that shit!
        {
            found+=14;
            string b;
            for(string::size_type i = found; i < a.length(); i++)
                b+=a.at(i);
            a=b;
        }
        return 14;
    }

    found = a.find("POLYGLOT"); //only in modd-ed polyglot
    if(found!=string::npos)
        return 15;

    return 0;
}

//take engine output, converts to... umm... yes *nods*
void Wrapper::readEngine(void)
{
    string readBuffer;
    while(1)
    {
        if(is_engineOutput() == 0) //if buffer is empty
        {
            readBuffer = getStream(); //read the stream
            if(!readBuffer.empty()) //if stream is full
            {
                int a = checkState(readBuffer, 1); //check the state, no trim
                if(a) //if the state is valid
                {
                    //set_engineOutput(readBuffer); //set the output
                    bool k;
                    do
                    {
                        k = set_engineOutput(readBuffer); //set the output
                    }
                    while(k);  //wait

                    printf("DEBUG: %s\n", readBuffer.c_str()); //comment me out
                }
            }
        }
        this_thread::sleep_for(chrono::milliseconds(HANDBREAK));
    }
}

//send "stuff" to the engine
void Wrapper::writeEngine(void)
{
    string input;
    while(1)
    {
        if(is_engineInput() == 1) //if the buffer is full
        {
            input = get_engineInput(); //get the input data
            putStream(input); //write the input data to the engine
        }
        this_thread::sleep_for(chrono::milliseconds(HANDBREAK)); //HANDBREAK!!!!!
    }
}

//read button input (cin emulation)
void Wrapper::readButton(void)
{
    char a;
    while(1)
    {
        if(is_button() == 0) //if no button is available
        {
            a = getchar();
            if(a)
            {
                //quick and dirty emulation for keypad
                //replace with real code later
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

                if(k!= -1)
                {
                    bool _k;
                    do
                    {
                        _k = set_button(k);
                    }
                    while(_k);  //wait
                }
            }
        }
        this_thread::sleep_for(chrono::milliseconds(HANDBREAK));
    }
}

//send data to the display (cout emulation)
void Wrapper::writeDispay(void)
{
    string disp;
    while(1)
    {
        if(is_display() == 1) //if the buffer is full
        {
            disp = get_display(); //get the display data
            printf("%s\n", disp.c_str()); //cout emu, display the data
            hwDisplay->print(disp); //writes to hardware display, nothing fancy
        }
        this_thread::sleep_for(chrono::milliseconds(HANDBREAK));
    }
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
    string buffer;
    proc->getline(buf, sizeof(buf), '\n');
    buffer = (buf);

    return buffer;
}

//clear stream, I think?
void Wrapper::clearStream(void)
{
    proc->clear();
}


// front end class for the wrapper
class frontEnd : public Wrapper
{
    using Wrapper::Wrapper;
public:
    void begin(void); //starts 9 million threads
    bool getUserMove(int a); //get user move
    void strUP(string &a); //string to uppercase
    void strLOW(string &a); //string to lowercase
    void easyEngine(string a); //send and wait
    void easyDisplay(string a); //send and wait
private:
    thread t1,t2,t3,t4;
    char alfa[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    char num[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
};

//start my awesome threads
void frontEnd::begin(void)
{
    t1 = thread(&frontEnd::readEngine, this);
    t2 = thread(&frontEnd::writeEngine, this);
    t3 = thread(&frontEnd::readButton, this);
    t4 = thread(&frontEnd::writeDispay, this);
}

//convert to upper case
void frontEnd::strUP(string &a)
{
    for(string::size_type i = 0; i < a.length(); i++)
    {
        if((a.at(i) >= 97) && (a.at(i) <= 122))
            a.at(i) = a.at(i)-32;
    }
}

//convert to lowercase
void frontEnd::strLOW(string &a)
{
    for(string::size_type i = 0; i < a.length(); i++)
    {
        if((a.at(i) >= 65) && (a.at(i) <= 90))
            a.at(i) = a.at(i)+32;
    }
}

//send to engine with force, just like Russia
void frontEnd::easyEngine(string a)
{
    bool k;
    do
    {
        k = set_engineInput(a);
    }
    while(k);  //wait
}

//send to display with force
void frontEnd::easyDisplay(string a)
{
    bool k;
    do
    {
        k = set_display(a);
    }
    while(k);  //wait
}

bool frontEnd::getUserMove(int a)
{
    string move = "----";
    move.at(0) = alfa[a-8]; //convert to indexed char
    int place = 1;
    bool waiting = 1;
    easyDisplay(move); //set_display(move); //send to display
    while(waiting)
    {
        bool _b = is_button();
        if(_b)
        {
            int _c = get_button();
            if((_c >= 8) && (_c <= 15) && (place < 4)) //buttons 8-15
            {
                if((place%2) == 1)
                    move.at(place) = num[_c-8];
                else
                    move.at(place) = alfa[_c-8];
                place++;
                easyDisplay(move); //set_display(move); //send to display
            }
            if(_c == 2) //clear
            {
                easyDisplay("----"); //send to display
                return 0;
            }
            if(_c == 3) //enter
            {
                strLOW(move); //convert to lower
                easyEngine(move); //send move
                easyDisplay("Hmmm"); //send to display
                waiting = 0;
                return 0;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(HANDBREAK));
    }
    return 0;
}

//error handling is for people who know what there doing
//warning! "no error handling"
//or... yay! no error handling!, renegade computing!

/*
		main need revamped, its just demo code anyway
		poop ta that!
*/

int main(void)
{
    printf("Chess Challenger\n");
    printf("Slash/Byte\n");
    printf("----------------\n");

    STARBURST HT; //hardware display
    frontEnd test(&HT); //polyglot + stockfish

    HT.begin(ADDR, 4); //address of the display and the number of digits
    HT.clrAll();
    HT.print("Chess Challenger", 150);
    HT.print("----");

    test.begin();

    bool _b;

    while(1)
    {
        _b = test.is_button();
        if(_b) //if there was a button press
        {
            int _c = test.get_button(); //get button
            if((_c >= 8) && (_c <= 15)) //buttons 8-15
            {
                //get user move
                test.getUserMove(_c); //first button press
            }
            else
            {
                //special function buttons
                if(_c == 2) //clear, CL
                {
                    test.easyDisplay("----"); //clears display
                    printf("clear\n");
                }
                if(_c == 7) //fen dump, PV
                {
                    test.easyEngine("DUMP"); //dumps the board
                }
                if(_c == 3) //EN
                {
                    test.easyEngine("go"); //test.set_engineInput("go"); //start the engine
                    test.easyDisplay("Hmmm"); //test.set_display("Hmmm");
                }
                if(_c == 0) //RE
                {
                    test.easyDisplay("New ");
                    test.easyEngine("new"); //new game
                    test.easyEngine("force"); //turns the clock off
                }
                if(_c == 4) //LV
                {
                    test.easyEngine("st 0"); //blitz time
                    printf("blitz on\n");
                }
                if(_c == 1) //CB
                {
                    test.easyEngine("easy"); //ponder off
                    printf("ponder off\n");
                }
                if(_c == 5) //DM
                {
                    test.easyEngine("hard"); //ponder on
                    printf("ponder on\n");
                }
            }
        }
        _b = test.is_engineOutput();
        if(_b) //if theres a response from the engine
        {
            string _d = test.get_engineOutput();
            int _c = test.checkState(_d); //what did it say?
            if(_c == 1)
                test.easyDisplay("Err "); //error
            if(_c == 2)
                test.easyDisplay("ILMV"); //illegal move
            if(_c == 12)
            {
                test.strUP(_d);
                test.easyDisplay(_d); //computers move
            }
        }
        this_thread::sleep_for(chrono::milliseconds(HANDBREAK)); //Tokyo drift!
    }
    return 0;
}
