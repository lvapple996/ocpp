

#include "ocppUtils.h"
#include <stdio.h>     //默认 printf等
#include <stdlib.h>     //随机数
#include <string.h>     //字符串操作函数
#include <time.h>    // time()


ocppUils::ocppUils()//const SCreateParams & rCreateParams)
//    : outerHandles_(rCreateParams.outerHandles_),
//      constParams_(rCreateParams.constParams_),

{
    
    memset(MsgUniqueSque,0x00,sizeof(MsgUniqueSque));
}

ocppUils::~ocppUils()
{
}


int ocppUils::mt_rand(int start, int end) 
{
    return rand() % (end + 1 - start) + start; /*生成一个[start,end)区间内的整数*/
}

void ocppUils::getMsgUniqueSque(char *dst)
{
   memcpy(dst,MsgUniqueSque,strlen(MsgUniqueSque));
}

void ocppUils::setMsgUniqueSque()
{
	srand((unsigned) (time(NULL)));
	int randNum = mt_rand(999, 10000);
	
	unsigned long long a = 0;
	time_t timer;   
    struct tm *tblock;
    time(&timer);
    tblock = gmtime(&timer);
    a = (tblock->tm_year+1900)*100;
    a = (a+tblock->tm_mon+1)*100;
    a = (a+tblock->tm_mday)*100;
    a = (a+tblock->tm_hour+8)*100;
    a = (a+tblock->tm_min)*100;
    a = (a+tblock->tm_sec);  
    
    sprintf(MsgUniqueSque, "%llu%d", a,randNum);
}

char *ocppUils::getStrchr(char *url,char chr)
{	
    char *p,*q;
    unsigned char tmp;
    p = strchr(url, chr);
    tmp = (unsigned char)(strchr(url, chr)-url);
    q = (char *)malloc(tmp);
    memcpy(q,url,tmp);
    return q; 
} 

/*功能：将str字符串中的字符串替换为newstr字符串
*参数：str：操作目标  oldstr：被替换者  newstr：替换者
*返回值：返回替dao换之后的字符串
*/
char*ocppUils::strrpc(char*str,char*oldstr,char*newstr)
{

  char bstr[strlen(str)];//转换缓冲区
  memset(bstr,0,sizeof(bstr));
  int i=0;
  for(;i<strlen(str);i++)
  {

    if(!strncmp(str+i,oldstr,strlen(oldstr)))//查找目标字符串
	{
       strcat(bstr,newstr);
       i+=strlen(oldstr)-1;
    }
	else
	{
       strncat(bstr,str+i,1);//保存一字节进缓冲区
    }
  }
  strcpy(str,bstr);
  return str;
}
/*
int main()
{
	char str[20];
	char *location = "ftp://1233.com";

	ocppUils *ocppUilsTest = new ocppUils();
	printf("%s",ocppUilsTest->getStrchr(location,':'));
	if(strcmp(ocppUilsTest->getStrchr(location,':'),"ftp") == 0)
	 printf("00000");
	return 0;
}
*/

