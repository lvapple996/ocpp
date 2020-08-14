#ifndef __OCPPCLIENT__HPP__
#define __OCPPCLIENT__HPP__


#include "easywsclient.hpp"
#include <pthread.h>
#include "msgQueue.h"


using easywsclient::WebSocket;
using namespace ocppMsgQueue;

namespace ocppClientJson{

class ocppClient{	
public:
    ocppClient(char *url = NULL);
    virtual~ocppClient();
    void ocppClientInit();
    static ocppClient *getInstance();

private:    
    //ocppClient *p_ocppClientInstance;
    OCPPMSGQueueProcess *queue;
    WebSocket::pointer ws;
    static ocppClient *ocpp_Instance;

    char wsUrl[128];
    pthread_t pidsend,pidpoll;
    pthread_mutex_t socketMutex;

    static void handle_message(const std::string & message);
    static void* msgProcessHandler(void *arg);
    static void* msgPollHandler(void *arg);
    int startClientThread();	
    void mSleep(int ms); 
};
}
#endif
