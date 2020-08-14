#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "ocppclient.h"
#include "ocppProtocl.h"


namespace ocppClientJson{

ocppClient * ocppClient::ocpp_Instance = NULL;


ocppClient::ocppClient(char *url)
{
    ocpp_Instance =this;
    if(url != NULL)
    {
        memcpy(wsUrl,url,strlen(url));
    }

    ocpp_Instance->queue = OCPPMSGQueueProcess::GetInstance();
    if(ocpp_Instance->queue->ocpp_msg_send_queue==NULL)
    {
        ocpp_Instance->queue->CreatQueue(0);
    }
}


ocppClient *ocppClient::getInstance()
{
	if(ocpp_Instance == NULL)
	   ocpp_Instance = new ocppClient();
	return ocpp_Instance;
}

ocppClient::~ocppClient()
{
    delete ocpp_Instance;
    
}

void ocppClient::mSleep(int ms) // o¨¢?????¨®¨º¡À
{
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &delay);
}


void* ocppClient::msgPollHandler(void *arg)
{ 
    WebSocket::pointer ws = (WebSocket::pointer)arg;
    while (ws->getReadyState() != WebSocket::CLOSED) {
      ws->poll();
      ws->dispatch(handle_message);
    }
}
void* ocppClient::msgProcessHandler(void *arg)
{ 
    WebSocket::pointer ws = (WebSocket::pointer)arg;

    OCPPMSGQueueNode* queueSend;
    while(1)
    {
        queueSend =ocpp_Instance->queue->DeQueue(ocpp_Instance->queue->ocpp_msg_send_queue);

        if(queueSend != NULL)
        {
            printf("queueSend >>> %s\n",(char *)queueSend->data);
            ws->send((char *)(queueSend->data));

            delete queueSend;
        } 
        ocpp_Instance->mSleep(100);
    }
}

void ocppClient::handle_message(const std::string & message)
{  

    if(message.c_str() != NULL)
    {
        OCPPMSGQueueNode *queueRecv = new OCPPMSGQueueNode;
        queueRecv->data = malloc(message.length()+1); 
        strcpy((char *)queueRecv->data,message.c_str());
        queueRecv->size = message.length()+1;
        queueRecv->index =1;
        printf("queueRecv <<< %s\n", message.c_str());

        ocpp_Instance->queue->EnQueue(ocpp_Instance->queue->ocpp_msg_recv_queue,queueRecv);
    }
    
}

void ocppClient::ocppClientInit()
{
	startClientThread();
}

int ocppClient::startClientThread()
{
    int ret=0;

	ws = WebSocket::from_url(wsUrl);
    assert(ws); 
    ret = pthread_create(&pidpoll, NULL, msgPollHandler, ws);
    if(ret)
    {
        printf("create pthread error!\n");
        return -1; 
    }
    ret = pthread_create(&pidsend, NULL, msgProcessHandler, ws);
    if(ret)
    {
        printf("create pthread error!\n");
        return -1; 
    }
    return 0;
}
/*
int main()
{
	printf("main \n");

	ocppClient *ocppClientObject = new ocppClient("172.16.5.172:6000");
	ocppClientObject->ocppClientInit();
	
	printf("connect success \n");

	ocppProtocl *ocppProtoclTest = new ocppProtocl();
	ocppProtoclTest->taskRun10ms();

	printf("report success \n");
	while(1);
	return 0;
}
*/
/*
int main()
{
	ocppProtocl *ocppProtoclTest = new ocppProtocl();
	ocppProtoclTest->taskRun10ms();

   //HeartbeatReq();
   //BootNotificationReq();
   //RemoteStartTransactionReq();
   //StartTransactionReq();
   //AuthorizeReq();
   //MeterValuesReq();
   //StopTransactionReq();
   //StatusNotificationReq();
   //UpdateFirmwareReq();
   //ResetReq();
   //GetDiagnosticsReq();
   //DiagnosticsStatusNotificationReq();
   //GetConfigurationReq();
   //ChangeConfigurationReq();
   //GetLocalListVersionReq();
   //FirmwareStatusNotificationReq();
   //ParseWebsocketJsonStr("[2,\"3\",\"RemoteStopTransaction\",{\"transactionId\":1}]");
   //ParseWebsocketJsonStr("[2,\"d7938ef3-bfc5-439e-beab-d1f665449e17\",\"Reset\",{\"type\":\"Hard\"}]");
   //ParseWebsocketJsonStr("[2,\"1285486790039572480\",\"UpdateFirmware\",{\"retrieveDate\":\"2020-07-21T08:08:28Z\",\"location\":\"http://www.baidu.com/Firmware/OCPPnoTouch.zip\"}]");
   //ParseWebsocketJsonStr("[2,\"1285486278514839552\",\"GetDiagnostics\",{\"stopTime\":\"2020-07-21T08:06:26Z\",\"location\":\"http://www.baidu.com/rizhi/1285486278560976896\"}]");
   //setOcppMsgSque("RemoteStartTransaction","12346789");
   //printf("\nmsgsque=%s",getOcppMsgSque("RemoteStartTransaction"));
   //enableStatusNotifFun(); 
}*/


}
