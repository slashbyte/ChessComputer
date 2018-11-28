/*
 * frontend.h
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
#ifndef frontend_h
#define frontend_h

// front end class for the wrapper
class FrontEnd : public Wrapper
{
    using Wrapper::Wrapper;
public:
    void begin(void); //starts 9 million threads
    bool is_button(void); //checks for valid button state
    int get_button(void); //gets button state
    bool is_engine(void); //checks for valid engine state
    std::string get_engine(void); //get engine state
    void send_display(std::string a); //send a string to the display
    void send_display(std::string a, bool b); //sends string to the display, b=1, to uppercase
    void send_engine(std::string a); //send to engine
    void strToUpper(std::string &a); //string to uppercase
    void strToLower(std::string &a); //string to lower case
    char buttonToChar(int a, int b); //converts a button code to a char
    /////////////////////////////////////
    void undoMove(void); //undo the last two moves
    void go(void); //invokes the engine
    void newGame(int sd); //starts a new game
    bool getMove(int b); //gets the users move
    void showMove(std::string b); //shows the current move
    void clear(void); //clears the hardware display
    void posVer(void); //position request
    void hint(void); //request a hint
    void error(void); //error screen
    void check(bool b); //set or unset check led
    void mate(bool b); //set or unset mate led
    void mateBlink(void); //blinks the screen if your in checkmate
    void thinkANI(void); //fancy think screen
    void think(void); //simple think screen
    void killScreen(int b, int _delay); //displays the kill screen (draw...)
    void fen(std::string b); //displays the fen string
    void saveDisplayState(void); //saves the current display state
    void restoreDisplayState(void); //restores the saved display state
    void level(int &sd); //sets the search depth level

private:
    std::thread t1,t2,t3,t4;
    const char alfa[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    const char num[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};
    const std::string spinner = "/-\\|";
    const std::string KS[7] = {"White Mates", "Black Mates", "Stalemate", "Draw", "Draw by 50 rule", "Draw", "Resign"};
    uint16_t mState[8];
};

#endif