#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

namespace httpGetPost{

#define SERVER_PORT 80	
class httpClient{
	
   public:
	  httpClient();
      ~httpClient();
	  int parseCjsonURL(char *cjsonURL);
      int httpGetFile();
      int httpPostFile();
	  
   private:
      char domainURL[128];//"aurora-uniev.oss-cn-beijing.aliyuncs.com"; 
      char domainPath[128];  //"/dev/dcharg.bin";
      char send_data[1024],recv_data[1024];   
	  
	  //get 
	  int ReadHttpStatus(int sock);
	  int ParseHeader(int sock);
	  
	  //post
	  int Connect_Server(char *severip,int port);

	  
};	
}



#endif