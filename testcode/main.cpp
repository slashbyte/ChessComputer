/*
 * main.cpp
 *
 * 	.▄▄ · ▄▄▌   ▄▄▄· .▄▄ ·  ▄ .▄▄▄▄▄·  ▄· ▄▌▄▄▄▄▄▄▄▄ .
 *  ▐█ ▀. ██•  ▐█ ▀█ ▐█ ▀. ██▪▐█▐█ ▀█▪▐█▪██▌•██  ▀▄.▀·
 *  ▄▀▀▀█▄██▪  ▄█▀▀█ ▄▀▀▀█▄██▀▐█▐█▀▀█▄▐█▌▐█▪ ▐█.▪▐▀▀▪▄
 *  ▐█▄▪▐█▐█▌▐▌▐█ ▪▐▌▐█▄▪▐███▌▐▀██▄▪▐█ ▐█▀·. ▐█▌·▐█▄▄▌
 *   ▀▀▀▀ .▀▀▀  ▀  ▀  ▀▀▀▀ ▀▀▀ ··▀▀▀▀   ▀ •  ▀▀▀  ▀▀▀
 *                                         11-28-2018
 *		Slash/Byte
 *	demo code for chess challenger
 */

#include <stdio.h>
#include <iostream>
#include <thread>
#include "pstream.h"

#include "ht16k33.h"
#include "starburst.h"
#include "threadobj.h"
#include "wrapper.h"
#include "frontend.h"

#define ADDR 0x70

using namespace std;

int main(void)
{
    printf("Chess Challenger\n");
    printf("Slash/Byte\n");
	//mapped the buttons to the keyboard for testing on the PC
	//still no hardware button code yet *sad*, slash/bytes lazy
    printf("----------------------\n"); 
    printf("Keyboard Emulation Map\n"); 
    printf("----------------------\n");
	printf("1:RE, 2:CB, 3:CL, 4:EN\n");
    printf("Q:LV, W:DM, E:PB, R:PV\n");
    printf("A:A1, S:B2, D:C3, F:D4\n");
    printf("Z:E5, X:F6, C:G7, V:H8\n");
    printf("----------------------\n");

    STARBURST HT; //hardware display
    FrontEnd CC(&HT); //polyglot + stockfish9 (or 8, 8's nice too... I guess)
    //thats right, I'm not smart enough to wright a "good" chess engine
    //but hey, that just means I can use "any" UCI compatible engine on my hardware!
	//probably.... 
    //assuming it works with polyglot (a UCI to win-board converter)
    //oh and polyglot supports opening books, so...
    //stockfish9 with an opening book, on hardware! *mind blown*
	//idk if stockfish supports books out of the box or not...

    HT.begin(ADDR, 4); //address of the display and the number of digits
    HT.clrAll();
    HT.print("SLASH/BTYE's Chess Challenger", 100);
	HT.print("StockFish 9", 100);

    //start the io threads, engine input/output & button/display IO, all independent
    CC.begin(); //not sure if that was the best decision, but it works well

    //helper variables for game play logic
    bool _b;
    bool SIDE = 0;
    bool NEWGAME = 1;
	bool PONDER = 0;
    const bool WHITE = 0;
    const bool BLACK = 1;
    int searchDepth = 10;
	
	CC.newGame(searchDepth, PONDER); //start new game

    while(1) //loop that will run till the end of time!, or till the batteries die...
    {
        _b = CC.is_button(); //check button
        if(_b) //if there was a button press
        {
            int _c = CC.get_button(); //get button
            if(_c == 0) //RE, reset
            {
                NEWGAME = 1; //reset game flag
                SIDE = 0; //reset side to default
                CC.mate(0); //clear mate
                CC.newGame(searchDepth, PONDER); //new game
            }
            if(_c == 1) //CB, challenger bottom, ponder enable disable?
				CC.ponder(PONDER);
            if(_c == 2) //CL, clear
                CC.clear(); //clear the HW display
            if(_c == 3) //EN, enter
            {
                if(NEWGAME)//if the game is new, flop sides
                {
                    SIDE=!SIDE;
                    NEWGAME=0;
                }
                CC.go(); //send engine command
                //CC.think(); //think screen
                CC.thinkANI(); //think with animation
            }
            if(_c == 4) //LV, level
                CC.level(searchDepth); //select your level of play
            if(_c == 5) //DM,"double move?" undo a move
			{
                CC.undoMove();
				CC.getBoard(); //debug stuff
			}
            if(_c == 6) //PB, problem mode
                CC.hint();
            if(_c == 7) //PV, position verification
                CC.posVer(); //position verification request
            if((_c >= 8)&&(_c <= 15)) //alpha-num buttons
            {
                if(NEWGAME)
                    NEWGAME=0;
                if(!CC.getMove(_c)) //if move was "entered"
                {
                    CC.check(0); //clear check led
                    //CC.think(); //show think screen
                    CC.thinkANI(); //think with animation
                }
            }
        }
        _b = CC.is_engine(); //check engine
        if(_b) //if theres a response from the engine
        {
            string _d = CC.get_engine(); //get engine response
            int _a = CC.checkState(_d, 1); //check and trim
			
            if((_a == 1) || (_a == 2)) //error or illegal move
                CC.error(); //error screen, flash @ 2hz
				
            if(_a == 3)//black in check
				if(SIDE == BLACK) //if my side
					CC.check(1); //set check led
					
            if(_a == 4) //white in check
				if(SIDE == WHITE) //if my side
					CC.check(1); //set check led
					
            if(_a == 5) //white mates
				if(SIDE == WHITE) //if not my side (computer)
					CC.mate(1); //set mate led
				
            if(_a == 6) //black mates
				if(SIDE == BLACK) //if not my side (computer)
					CC.mate(1); //set mate led
				
            if((_a == 5) || (_a == 6))
                CC.mateBlink(); //flash display @ 1hz, if mate (end of game)
			
            if((_a >= 7) && (_a <= 11)) //scroll kill-screen (draw message)
                CC.killScreen(_a, 150); //@150ms/char speed
			
            if(_a == 12) //get computer move
			{
				printf("COMPUTER: %s\n", _d.c_str()); //for debug, and stuff, comment me out!
                CC.showMove(_d); //display the computers move
				CC.getBoard(); //debug stuff
			}
			//"hint" wont work in "level 01", IDK why
			//I think it has something to do with the "super limited" search depth
            if(_a == 13) //hint
			{
				printf("HINT: %s\n", _d.c_str()); //for debug, and stuff, comment me out!
                CC.showHint(_d); //display computer hint, in different notation.
			}
            if(_a == 14) //fen
			{
				printf("FEN: %s\n", _d.c_str()); //for debug, and stuff, comment me out!
                CC.fen(_d); //display fen string
			}
			if(_a == 15) //board output
				CC.printBoard(_d);				
        }
        CC.cpuBreak();
    }
    return 0;
}
