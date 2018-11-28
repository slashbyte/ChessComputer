/*
 * wrapper.h
 *
 * 	.▄▄ · ▄▄▌   ▄▄▄· .▄▄ ·  ▄ .▄▄▄▄▄·  ▄· ▄▌▄▄▄▄▄▄▄▄ .
 *  ▐█ ▀. ██•  ▐█ ▀█ ▐█ ▀. ██▪▐█▐█ ▀█▪▐█▪██▌•██  ▀▄.▀·
 *  ▄▀▀▀█▄██▪  ▄█▀▀█ ▄▀▀▀█▄██▀▐█▐█▀▀█▄▐█▌▐█▪ ▐█.▪▐▀▀▪▄
 *  ▐█▄▪▐█▐█▌▐▌▐█ ▪▐▌▐█▄▪▐███▌▐▀██▄▪▐█ ▐█▀·. ▐█▌·▐█▄▄▌
 *   ▀▀▀▀ .▀▀▀  ▀  ▀  ▀▀▀▀ ▀▀▀ ··▀▀▀▀   ▀ •  ▀▀▀  ▀▀▀
 *                                         11-28-2018
 *
 */

#ifndef wrapper_h
#define wrapper_h

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
    int checkState(std::string &a, bool b = 0); //get state code from string
    void cpuBreak(void)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(HANDBREAK));
    };
    //string getState(int a){return response[a-1];};
private:
    redi::pstream* proc;
    const std::string response[15] =
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
    void putStream(std::string a);
    std::string getStream(void);
    void clearStream(void);
    const int HANDBREAK = 5;
};

#endif