#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include "httpClient.h"

namespace httpGetPost{
char upload_head[] =
	"POST %s HTTP/1.1\r\n"
	"Host: %s:%d\r\n"
	"Connection: keep-alive\r\n"
	"Content-Type: multipart/form-data; boundary=%s\r\n"
	"Content-Length: %d\r\n\r\n"
	"--%s\r\n"
	"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
	"Content-Type: application/octet-stream;chartset=UTF-8\r\n\r\n";
	
char upload_request[] = 
	"--%s\r\n"
	"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
	"Content-Type: application/octet-stream;chartset=UTF-8\r\n\r\n";
	
httpClient::httpClient()
{
  printf("create http obj\n\n");
  
}
httpClient::~httpClient()
{
  printf("destory http obj\n\n");
}
//http://aurora-uniev.oss-cn-beijing.aliyuncs.com/dev/dcharg.bin
//http://unicronlogic-rest.didichuxing.com/logUpload	
int httpClient::parseCjsonURL(char *cjsonURL)
{
   	if(cjsonURL == NULL)
	{
		return -1;
	}
    char urlToUrl[128] = {0};
	memcpy(urlToUrl,cjsonURL+7,strlen(cjsonURL)-7);
	int i = 0; 
	for(; i < strlen(urlToUrl);i++ )
	{
	   if(urlToUrl[i] == '/')
	   {
	   	  memcpy(domainURL,urlToUrl,i);
	   	  memcpy(domainPath,urlToUrl+i,strlen(cjsonURL)-7-i);
		  printf("domainURL %s domainPath %s\n\n",domainURL,domainPath);
	   	  break;
	   }
	} 
	
	return 0;
}

	
int httpClient::ReadHttpStatus(int sock)
{
    char c;
    char buff[1024]="",*ptr=buff+1;
    int bytes_received, status;
    
    printf("Begin Response ..\n");
    bytes_received = recv(sock, ptr, 1, 0);
    while(bytes_received)
    {
      if(bytes_received==-1)
      {
        perror("ReadHttpStatus");
        exit(1);
      }
 
      if((ptr[-1]=='\r') && (*ptr=='\n' )) break;
      ptr++;
      bytes_received = recv(sock, ptr, 1, 0);
    }
    *ptr=0;
    ptr=buff+1;
 
    sscanf(ptr,"%*s %d ", &status);
 
    printf("%s\n",ptr);
    printf("status=%d\n",status);
    printf("End Response ..\n");
    return (bytes_received>0)?status:0;
 
}
 
//the only filed that it parsed is 'Content-Length' 
int httpClient::ParseHeader(int sock)
{
    char c;
    char buff[1024]="",*ptr=buff+4;
    int bytes_received, status;
    
    bytes_received = recv(sock, ptr, 1, 0);
    printf("Begin HEADER ..\n");
    while(bytes_received)
    {
      if(bytes_received==-1)
      {
          perror("Parse Header");
          exit(1);
      }
 
      if(
          (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
          (ptr[-1]=='\r')  && (*ptr=='\n' )
      ) break;
      ptr++;
      bytes_received = recv(sock, ptr, 1, 0);
    }
 
    *ptr=0;
    ptr=buff+4;
    //printf("%s",ptr);
 
    if(bytes_received)
    {
      ptr=strstr(ptr,"Content-Length:");
      if(ptr)
      {
        sscanf(ptr,"%*s %d",&bytes_received);
      }
      else
        bytes_received=-1; //unknown size
 
     	printf("Content-Length: %d\n",bytes_received);
    }
    printf("End HEADER ..\n");
    return  bytes_received ;
 
}
 

int httpClient::httpGetFile(void)
{
    int sock, bytes_received;  
    char*p;
    struct sockaddr_in server_addr;
    struct hostent *he;
    float processbar = 0.0f;
 
    he = gethostbyname(domainURL);
    if (he == NULL)
    {
       herror("gethostbyname");
       exit(1);
    }
 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
       perror("Socket");
       exit(1);
    }
    server_addr.sin_family = AF_INET;     
    server_addr.sin_port = htons(SERVER_PORT);//httpĬ�϶˿�80
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(server_addr.sin_zero),8); 
 
    printf("Connecting ...\n");
    if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
    {
       perror("Connect");
       exit(1); 
    }
 
    printf("Sending data ...\n");
 
    snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", domainPath, domainURL);

    if(send(sock, send_data, strlen(send_data), 0)==-1)
    {
        perror("send");
        exit(2); 
    }
    printf("Data sent.\n");  
 
    //fp=fopen("received_file","wb");
    printf("Recieving data...\n\n");
 
    int contentlengh;
 
    if(ReadHttpStatus(sock) && (contentlengh=ParseHeader(sock)))
    {
      int bytes=0;
      FILE* fd=fopen("dcharg.bin","wb");
      printf("Saving data...\n\n");
	  
	  bytes_received = recv(sock, recv_data, 1024, 0);
      while(bytes_received)
      {
		if(bytes_received==-1)
		{
		  perror("recieve");
		  exit(3);
		}
        fwrite(recv_data,1,bytes_received,fd);
        bytes+=bytes_received;
   
		processbar = (float)bytes / (float)contentlengh;
        printf("%6d/%lu,%2.1f%\n", bytes, contentlengh, processbar * 100);
		
        if(bytes==contentlengh)
        	break;
        bytes_received = recv(sock, recv_data, 1024, 0);
      }
      fclose(fd);
    }
 
 
 
    close(sock);
    printf("\n\nDone.\n\n");
    return 0;
}

int httpClient::Connect_Server(char *severip,int port)
{
	int sock = -1;
	struct sockaddr_in addr;
	struct hostent *he;
	
	if(severip == NULL || port <= 0)
		return -1;
	
	he = gethostbyname(severip);
    if (he == NULL)
    {
       printf("gethostbyname  error");
       return -1;
    }	
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//inet_pton(AF_INET,severip,&addr.sin_addr.s_addr);
	addr.sin_addr = *((struct in_addr *)he->h_addr);
	
	bzero(&(addr.sin_zero),8);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
       printf("new Socket error");
       return -1;
    }
	
	if (connect(sock, (struct sockaddr *)&addr,sizeof(struct     sockaddr)) == -1)
    {
       printf("Connect Socket error");
       close(sock);
	   return -1;
    }
	return sock;	
	
}

int httpClient::httpPostFile()
{
	char *filename = "test.txt";
	int sock = -1;
	char send_date[4096] = {0};
	char send_end[128] = {0};
	char send_request[2048] = {0};
	FILE *fp = NULL;
	//char *userid = "00000000199";
	char boundary[64] = {0};
	int ret = -1,already = -1,ContentLength = -1;
	long long int timestamp;
	struct timeval tv;


	if(access(filename,F_OK) != 0)
	{
		printf("file %s not exsit!\n",filename);
		return -1;
	}

    //连接服务器
	sock = Connect_Server(domainURL,SERVER_PORT);
	if(sock < 0 )
	{
		printf("connect server error!\n");
		return -1;
	}
	
    //打开要上传的文件，获取文件的大小，用于计算Content-Length的大小
	fp = fopen(filename,"rb");
	if(fp == NULL)
	{
		printf("open file %s error!\n",filename);
		close(sock);
		return -1;
	}
	
	fseek(fp,0,SEEK_END);
	ContentLength = ftell(fp);
	rewind(fp);
	
    //获取毫秒级的时间戳用于boundary的值
	gettimeofday(&tv,NULL);
	timestamp = (long long int)tv.tv_sec * 1000 + tv.tv_usec;
	snprintf(boundary,64,"---------------------------%lld",timestamp);
	
    // Content-Length的大小还包括了对上传文件的描述，开始boundary和结束boundary
	ContentLength += snprintf(send_request,2048,upload_request,boundary,filename);
	ContentLength +=snprintf(send_end,2048,"\r\n--%s--\r\n",boundary);
	
    //发送的http头和上传文件的描述
	ret = snprintf(send_date,4096,upload_head,domainPath,domainURL,SERVER_PORT,boundary,ContentLength,boundary,filename);

	if(send(sock,send_date,ret,0) != ret)
	{
		printf("send head error!\n");
		close(sock);
		return -1;
	}
	
//循环读取文件，并往服务器上面发送，直到文件结束为止
	clearerr(fp);
	while(1)
	{
			memset(send_date,0,sizeof(send_date));
			ret = fread(send_date,1,4096,fp);
			if(ret != 4096)
			{
					if(!ferror(fp))
					{
						if(send(sock,send_date,ret,0) != ret)
						{
								printf("send the end date error!\n");
								close(sock);
								fclose(fp);
								return -1;
						}
						fclose(fp);
						break;
					}
					else
					{
						printf("read file error!\n");
						close(sock);
						fclose(fp);
						return -1;
					}
						
			}
			
			if(send(sock,send_date,4096,0) != 4096)
			{
					printf("send date error\n");
					close(sock);
					fclose(fp);
					return -1;
			}
			
	}
//发送最后的boundary结束文件上传。
SEND_END:		
	memset(send_date,0,sizeof(send_date));
	ret = snprintf(send_date,4096,"\r\n--%s--\r\n",boundary);
	if(send(sock,send_date,ret,0) != ret)
	{
			close(sock);
			return -1;
	}
	
	printf("send to server end date:%s\n",send_date);
//接收返回值，用于判断是否上传成功
	memset(send_date,0,sizeof(send_date));
	if(recv(sock,send_date,4096,0) < 0)
		printf("recv error!\n");
		
	printf("recv:%s\n",send_date);
	close(sock);
	
	return 0;
}
}