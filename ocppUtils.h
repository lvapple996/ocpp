#ifndef __OCPPUTILS_H__
#define __OCPPUTILS_H__


class ocppUils
{
public:
    ocppUils();
    ~ocppUils();
    char*strrpc(char*str,char*oldstr,char*newstr);
    void setMsgUniqueSque(); 
    void getMsgUniqueSque(char *dst);
    int mt_rand(int start, int end);
    char *getStrchr(char *url,char chr);
    char MsgUniqueSque[18];

private:


};	

#endif
