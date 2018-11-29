/*
 * threadobj.h
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

#ifndef threadobj_h
#define threadobj_h

class ThreadBuffer
{
public:
    ThreadBuffer() {};
    //buffer access
    bool is(void);
    bool set_str(std::string a);
    bool set_int(int a);
    void set_str_easy(std::string a);
    void set_int_easy(int a);
    std::string get_str(void);
    int get_int(void);
	
private:
    //buffer
    bool _is = 0;
    std::string strBuf;
    int intBuf = 0;
};

#endif