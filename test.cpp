#include <assert.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>
#include "ocppclient.h"
#include "ocppProtocl.h"
#include "msgQueue.h"
int main()
{
	printf("main \n");
    using namespace ocppClientJson; 
    using namespace ocppMsgQueue;
    using namespace ocppProtoclHand;



	OCPPMSGQueueProcess *queuePro = OCPPMSGQueueProcess::GetInstance();
	queuePro->CreatQueue(0);
    
	ocppClient *ocppClientObject = new ocppClient((char *)"ws://central-nsna.voltiosc.cl/sinexcel01");
	ocppClientObject->ocppClientInit();
	//printf("connect success \n");

	ocppProtocl *ocppProtoclTest = new ocppProtocl();
    ocppProtoclTest->ocppProtoclInit();

	//printf("report success \n");
	while(1);
	return 0;
	
}
