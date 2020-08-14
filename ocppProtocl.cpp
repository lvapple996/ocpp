
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/prctl.h>

#include "cJSON.h"
#include "ocppProtocl.h"


#define CALL_VAR(i) pJsonSub_3_##i
#define false 0
#define true 1
#define REQUEST  2  //��Ϣ���� 
#define RESPONSE 3  //��Ϣ��Ӧ 
#define HEART_PERIOD  10 //��λ:s 
#define HEART_TIMEOUT_CNT  5 //����ʱ���� 5


//�豸��Ϣ����
namespace ocppProtoclHand{

//���к�Ĭ����16λ0 
OcppProtoclReq ocppProtocl::OcppProtoclReqArry[SHELL_INFO_NUM_MAX] = {
  {0,(char *)"00000000000000000",(char *)"BootNotification",false,true,HEART_PERIOD,ParseBootNotificationReq,BootNotificationReq},
  {1,(char *)"00000000000000000",(char *)"Heartbeat",false,false,HEART_PERIOD,ParseHeartbeatReq,HeartbeatReq},
  {2,(char *)"00000000000000000",(char *)"RemoteStartTransaction",true,false,0,ParseRemoteStartTransactionReq,RemoteStartTransactionReq},
  {3,(char *)"00000000000000000",(char *)"Authorize",false,false,0,ParseAuthorizeReq,AuthorizeReq},
  {4,(char *)"00000000000000000",(char *)"StartTransaction",false,false,0,ParseStartTransactionReq,StartTransactionReq},
  {5,(char *)"00000000000000000",(char *)"MeterValues",false,false,0,ParseMeterValuesReq,MeterValuesReq},
  {6,(char *)"00000000000000000",(char *)"StopTransaction",false,false,0,ParseStopTransactionReq,StopTransactionReq},
  {7,(char *)"00000000000000000",(char *)"StatusNotification",false,false,0,ParseStatusNotificationReq,StatusNotificationReq},
  {8,(char *)"00000000000000000",(char *)"RemoteStopTransaction",true,false,0,ParseRemoteStopTransactionReq,RemoteStopTransactionReq},
  {9,(char *)"00000000000000000",(char *)"Reset",true,false,0,ParseResetReq,ResetReq},
  {10,(char *)"00000000000000000",(char *)"GetDiagnostics",true,false,0,ParseGetDiagnosticsReq,GetDiagnosticsReq},
  {11,(char *)"00000000000000000",(char *)"UpdateFirmware",true,false,0,ParseUpdateFirmwareReq,UpdateFirmwareReq},
  {12,(char *)"00000000000000000",(char *)"ChangeConfiguration",true,false,0,ParseChangeConfigurationReq,ChangeConfigurationReq},
  {13,(char *)"00000000000000000",(char *)"ChangeAvailability",true,false,0,ParseChangeAvailabilityReq,ChangeAvailabilityReq},
  {14,(char *)"00000000000000000",(char *)"GetConfiguration",true,false,0,ParseGetConfigurationReq,GetConfigurationReq},
  {15,(char *)"00000000000000000",(char *)"GetLocalListVersion",true,false,0,ParseGetLocalListVersionReq,GetLocalListVersionReq},
};
/******************************************************************************************/

ocppProtocl * ocppProtocl::ocpp_Instance = NULL;

ocppProtocl *ocppProtocl::getInstance()
{
	if(ocpp_Instance == NULL)
	   ocpp_Instance = new ocppProtocl();
	return ocpp_Instance;
}

ocppProtocl::ocppProtocl()
    :HeartStatus(false),
    BootNotifiStatus(false), //false:δע��  true��ע��ɹ�
    HeartbeatInterv(HEART_PERIOD), //Ĭ��������� 10s
    HeartbeatTimeOutCnt(0),//������ʱ���� 0
    ChargerPointStatus(1)    //���׮�豸״̬ 0:����  1�����  2��������  3������
    //const SCreateParams & rCreateParams)
//    : outerHandles_(rCreateParams.outerHandles_),
//      constParams_(rCreateParams.constParams_),
{
    ocpp_Instance =this;
    queue = OCPPMSGQueueProcess::GetInstance();
    if(ocpp_Instance->queue->ocpp_msg_send_queue==NULL)
    {
        ocpp_Instance->queue->CreatQueue(0);
    }
	ocppUilsChange = new ocppUils();
   	httpClientobject = new httpClient();
	
}

ocppProtocl::~ocppProtocl()
{
    delete ocppUilsChange;
    delete ocpp_Instance;
}

void ocppProtocl::mSleep(int ms) // ���뼶��ʱ
{
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = ms * 1000;
    select(0, NULL, NULL, NULL, &delay);
}


/**************************************�ṹ�������������*****************************************/
int ocppProtocl::getOcppProtoclReqArrySize()
{
	return sizeof(OcppProtoclReqArry)/sizeof(OcppProtoclReq);
}

int ocppProtocl::getOcppMsgArryId(char *pMsgId,char *pMsgSque)
{
	int i = 0 ;
	
	int iArrySize =  getOcppProtoclReqArrySize();

	if(pMsgId == NULL&& pMsgSque == NULL)
	{
	   return -1;	
	}
	
	if(pMsgId != NULL && pMsgSque != NULL)
	{
	   return -2;
	}
	
    if(pMsgId != NULL)
	{
	   for(;i < iArrySize; i++)
	   {
	      if(strncmp(pMsgId,OcppProtoclReqArry[i].pMsgId,strlen(pMsgId)) == 0) 
	      {
	     	return i;
		  }
	    } 
	}
	
	if(pMsgSque != NULL)
	{
	   for(;i < iArrySize; i++)
	   {
          if(strncmp(pMsgSque,OcppProtoclReqArry[i].pMsgSque,strlen(pMsgSque)) == 0) 
	      {
	     	return i;
		  }
	    } 	
	}

	return -3;
}

int ocppProtocl::setOcppMsgSque(char *pMsgId,char *pMsgSque)
{
	 int iArrySize =  getOcppProtoclReqArrySize();
	 int i = 0;
	 for(;i < iArrySize; i++)
	 {
	     if(memcmp(pMsgId,OcppProtoclReqArry[i].pMsgId,strlen(pMsgId)) == 0) 
	     {
	     	OcppProtoclReqArry[i].pMsgSque=pMsgSque;
	     	return 0;
	     	/*
			 OcppProtoclReqArry[i].pMsgSque = (char *)malloc(strlen(pMsgSque)+1);
	     	if(OcppProtoclReqArry[i].pMsgSque == NULL)
	     	{
	     		return;
			}
	     	strcpy(OcppProtoclReqArry[i].pMsgSque,pMsgSque);
	     	*/
		 }
	 }
	 return -1; 	
}

char *ocppProtocl::getOcppMsgSque(char *pMsgId)
{
	 int iArrySize =  getOcppProtoclReqArrySize();
	 int i = 0;
	 for(;i < iArrySize; i++)
	 {
	     if(memcmp(pMsgId,OcppProtoclReqArry[i].pMsgId,strlen(pMsgId)) == 0) 
	     {
	     	return OcppProtoclReqArry[i].pMsgSque;
		 }
	 }
	 return NULL; 	
}

int ocppProtocl::setOcppMsgIsRequst(char *pMsgId,bool isRequst)
{
	if(pMsgId == NULL)
	{
		return -1;
	}
	int MsgArryId = getOcppMsgArryId(pMsgId,NULL);
	if(MsgArryId < 0)
	{
	   return -2;	
	}
	OcppProtoclReqArry[MsgArryId].isRequst = isRequst;
	return 0;
}

int ocppProtocl::setOcppMsgIsSend(char *pMsgId,bool isSend)
{
	if(pMsgId == NULL)
	{
		return -1;
	}
	int MsgArryId = getOcppMsgArryId(pMsgId,NULL);

	if(MsgArryId < 0)
	{
	   return -2;	
	}
	OcppProtoclReqArry[MsgArryId].isSend = isSend;
	return 0;	
}

int ocppProtocl::setOcppMsgPeriod(char *pMsgId,bool period)
{
	if(pMsgId == NULL)
	{
		return -1;
	}
	int MsgArryId = getOcppMsgArryId(pMsgId,NULL);
	if(MsgArryId < 0)
	{
	   return -2;	
	}
	OcppProtoclReqArry[MsgArryId].period = period;
	return 0;	
}

int ocppProtocl::getOcppMsgPeriod(int Id)
{
	if(Id < 0)
	{
		return -1;
	}
    return	OcppProtoclReqArry[Id].period ;
}

/*******************************************************************************************/
/**************************************��Ϣ������*****************************************/
//�豸�ϴ���Ϣ�崦���� 
char *ocppProtocl::BootNotificationReq()
{
    cJSON  *pJsonArry,*pJsonsub;
    static char msgSque[18] = {0};
	ocpp_Instance->ocppUilsChange->setMsgUniqueSque();
	memcpy(msgSque,ocpp_Instance->ocppUilsChange->MsgUniqueSque,18);
    ocpp_Instance->setOcppMsgSque((char *)"BootNotification",msgSque);
	printf("set BootNotificationReq msgSque��%s\n",ocpp_Instance->getOcppMsgSque((char *)"BootNotification"));

    pJsonArry=cJSON_CreateArray();   /*��������*/
    cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/

    cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString(msgSque)); /* ���������������ӶԶ���*/   
    cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("BootNotification")); /* ���������������ӶԶ���*/   
    cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
    cJSON_AddStringToObject(pJsonsub, "iccid","null"); 
    cJSON_AddStringToObject(pJsonsub, "firmwareVersion","V1.3.88_OCPP_beta2"); 
    cJSON_AddStringToObject(pJsonsub, "imsi",""); 
    cJSON_AddStringToObject(pJsonsub, "chargePointModel","C6EU");  
    cJSON_AddStringToObject(pJsonsub, "chargePointSerialNumber","C6E1XXXX18XXXXF");
    cJSON_AddStringToObject(pJsonsub, "chargeBoxSerialNumber","ADFCYYEAELF");
    cJSON_AddStringToObject(pJsonsub, "chargePointVendor","XCHARGE");
                                                                                        
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
    printf("BootNotificationReq OcppProtoclReqArry[0].pMsgSque = %s\n",OcppProtoclReqArry[0].pMsgSque);

   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);	
   return 	pJson;
}

char *ocppProtocl::HeartbeatReq()
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   ocpp_Instance->ocppUilsChange->setMsgUniqueSque();
   ocpp_Instance->setOcppMsgSque((char *)"Heartbeat",ocpp_Instance->ocppUilsChange->MsgUniqueSque);
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString(ocpp_Instance->ocppUilsChange->MsgUniqueSque)); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("Heartbeat")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
                                                                                               
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);	
   return 	pJson;
} 

//[3, "3", {"status":"Accepted"}]
char *ocppProtocl::RemoteStartTransactionReq()
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("3")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "status","Accepted");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;	
}

char *ocppProtocl::RemoteStopTransactionReq()
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("3")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "status","Accepted");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

char *ocppProtocl::AuthorizeReq()
{
	
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1281962846321139712")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("Authorize")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   cJSON_AddStringToObject(pJsonsub, "idTag","00168108abea");                                                                                            
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   
   cJSON_Delete(pJsonArry);	
   return 	pJson;	
}

char *ocppProtocl::StartTransactionReq()
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1594865834578")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("StartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   cJSON_AddNumberToObject(pJsonsub, "connectorId",1);
   cJSON_AddStringToObject(pJsonsub, "idTag","3"); 
   cJSON_AddNumberToObject(pJsonsub, "meterStart",0); 
   cJSON_AddStringToObject(pJsonsub, "timestamp","2020-07-16T02:17:14Z");  
                                                                                            
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   
   cJSON_Delete(pJsonArry);		
   return 	pJson;	
}

char *ocppProtocl::MeterValuesReq()
{
   cJSON  *pJsonArry,*pJsonsub,*pJsonSub_0,*pJsonSub_1,*pJsonSub_2; 
   cJSON  *pJsonSub_3_0,*pJsonSub_3_1,*pJsonSub_3_2,*pJsonSub_3_3,*pJsonSub_3_4,*pJsonSub_3_5,*pJsonSub_3_6 ;
    
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1281962987467857920")); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("MeterValues")); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/  
   cJSON_AddNumberToObject(pJsonsub, "connectorId",1);
   
   cJSON_AddItemToObject(pJsonsub,"meterValue",pJsonSub_0=cJSON_CreateArray()); 
   cJSON_AddItemToObject(pJsonSub_0,"timestamp", pJsonSub_1=cJSON_CreateObject());
   cJSON_AddStringToObject(pJsonSub_1, "timestamp","2020-07-16T02:17:14Z");
   
   cJSON_AddItemToObject(pJsonSub_1,"sampledValue",pJsonSub_2=cJSON_CreateArray()); 
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_0=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_0, "context","Transaction.Begin");
   cJSON_AddStringToObject(pJsonSub_3_0, "value","49602.23");
   cJSON_AddStringToObject(pJsonSub_3_0, "unit","Wh");
   cJSON_AddStringToObject(pJsonSub_3_0, "measurand","Energy.Active.Import.Register");
   cJSON_AddStringToObject(pJsonSub_3_0, "location","Outlet");

   
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_1=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_1, "context","Transaction.Begin");
      cJSON_AddStringToObject(pJsonSub_3_1, "value","650.1");
   cJSON_AddStringToObject(pJsonSub_3_1, "unit","V");
   cJSON_AddStringToObject(pJsonSub_3_1, "measurand","Voltage");
   cJSON_AddStringToObject(pJsonSub_3_1, "location","Outlet");
   
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_2=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_2, "context","Transaction.Begin");
      cJSON_AddStringToObject(pJsonSub_3_2, "value","49602.62");
   cJSON_AddStringToObject(pJsonSub_3_2, "unit","W");
   cJSON_AddStringToObject(pJsonSub_3_2, "measurand","Power.Active.Import");
   cJSON_AddStringToObject(pJsonSub_3_2, "location","Outlet");
   
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_3=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_3, "context","Transaction.Begin");
      cJSON_AddStringToObject(pJsonSub_3_3, "value","56");
   cJSON_AddStringToObject(pJsonSub_3_3, "unit","Percent");
   cJSON_AddStringToObject(pJsonSub_3_3, "measurand","SoC");
   cJSON_AddStringToObject(pJsonSub_3_3, "location","Outlet");
   
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_4=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_4, "context","Transaction.Begin");
      cJSON_AddStringToObject(pJsonSub_3_4, "value","76.30");
   cJSON_AddStringToObject(pJsonSub_3_4, "unit","A");
   cJSON_AddStringToObject(pJsonSub_3_4, "measurand","Current.Offered");
   cJSON_AddStringToObject(pJsonSub_3_4, "location","Outlet");
   
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_5=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_5, "context","Transaction.Begin");
      cJSON_AddStringToObject(pJsonSub_3_5, "value","76.30");
   cJSON_AddStringToObject(pJsonSub_3_5, "unit","A");
   cJSON_AddStringToObject(pJsonSub_3_5, "measurand","Current.Import");
   cJSON_AddStringToObject(pJsonSub_3_5, "location","Outlet");
   
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_6=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_6, "context","Transaction.Begin");
       cJSON_AddStringToObject(pJsonSub_3_6, "value","45");
   cJSON_AddStringToObject(pJsonSub_3_6, "unit","Celsius");
   cJSON_AddStringToObject(pJsonSub_3_6, "measurand","Temperature");
   cJSON_AddStringToObject(pJsonSub_3_6, "location","Outlet");
	
	cJSON_AddNumberToObject(pJsonsub, "transactionId",879); 
	                                                                                         
   char * pJson = cJSON_Print(pJsonArry);
   printf("%s\n",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   
   cJSON_Delete(pJsonArry);		
	return 	pJson;
}
 
char *ocppProtocl::StopTransactionReq()
{
   cJSON  *pJsonArry,*pJsonsub,*pJsonSub_0,*pJsonSub_1,*pJsonSub_2; 
   cJSON  *pJsonSub_3_0,*pJsonSub_3_1,*pJsonSub_3_2,*pJsonSub_3_3,*pJsonSub_3_4,*pJsonSub_3_5,*pJsonSub_3_6 ;
    
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1281962987467857920")); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("MeterValues")); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/  
   cJSON_AddStringToObject(pJsonsub, "timestamp","2020-07-16T02:17:14Z");
   cJSON_AddNumberToObject(pJsonsub, "meterStop",97283);
   
   cJSON_AddItemToObject(pJsonsub,"transactionData",pJsonSub_0=cJSON_CreateArray()); 
   cJSON_AddItemToObject(pJsonSub_0,"timestamp", pJsonSub_1=cJSON_CreateObject());
   cJSON_AddStringToObject(pJsonSub_1, "timestamp","2020-07-16T02:17:14Z");
   
   cJSON_AddItemToObject(pJsonSub_1,"sampledValue",pJsonSub_2=cJSON_CreateArray()); 
   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_0=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonSub_3_0, "value","49602.23");
   cJSON_AddStringToObject(pJsonSub_3_0, "unit","Wh");
   cJSON_AddStringToObject(pJsonSub_3_0, "measurand","Energy.Active.Import.Register");

   cJSON_AddItemToArray(pJsonSub_2,pJsonSub_3_1=cJSON_CreateObject()); 
      cJSON_AddNumberToObject(pJsonSub_3_1, "value",85);
   cJSON_AddStringToObject(pJsonSub_3_1, "unit","Percent");
   cJSON_AddStringToObject(pJsonSub_3_1, "measurand","SoC");
   	
	cJSON_AddNumberToObject(pJsonsub, "transactionId",879);
	cJSON_AddStringToObject(pJsonsub, "idTag","00168108abea");
	cJSON_AddStringToObject(pJsonsub, "reason","Other");
	char * pJson = cJSON_Print(pJsonArry);
	   
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   
   cJSON_Delete(pJsonArry);
   return pJson;
}

char *ocppProtocl::StatusNotificationReq()
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   ocpp_Instance->ocppUilsChange->setMsgUniqueSque();
   ocpp_Instance->setOcppMsgSque((char *)"StatusNotification",ocpp_Instance->ocppUilsChange->MsgUniqueSque);
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString(ocpp_Instance->ocppUilsChange->MsgUniqueSque));
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("StatusNotification"));   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); 
   cJSON_AddStringToObject(pJsonsub, "vendorId","xcharge");
   cJSON_AddStringToObject(pJsonsub, "vendorErrorCode","0000"); 
   cJSON_AddStringToObject(pJsonsub, "errorCode","NoError"); 
   cJSON_AddStringToObject(pJsonsub, "status","Available");  
                                                                                            
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   
   cJSON_Delete(pJsonArry);	
   return 	pJson;		
}

//[3, "1285486790039572480", {}]
char *ocppProtocl::UpdateFirmwareReq()//�豸��Ӧƽ̨���� 
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1285486790039572480")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   //cJSON_AddStringToObject(pJsonsub, "status","Accepted");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}
 
//2, "1595318924363", "FirmwareStatusNotification", {"status":"Downloaded"}]
char *ocppProtocl::FirmwareStatusNotificationReq()//�豸�ϱ������ļ�״̬
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1595318924363")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("FirmwareStatusNotification")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "status","Downloading");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

//[3, "1285486278514839552", {"fileName":"20131212.log"}]
char *ocppProtocl::GetDiagnosticsReq()//�豸��Ӧƽ̨��־���� 
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1285486278514839552")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "fileName","bms.log");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

//[2, "1595318808710", "DiagnosticsStatusNotification", {"status":"Uploading"}]
char *ocppProtocl::DiagnosticsStatusNotificationReq()//�豸�ϴ��ļ�״̬
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(2,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1595318808710")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("DiagnosticsStatusNotification")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "status","Uploading");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

char *ocppProtocl::ResetReq()//�豸��Ӧƽ̨�·�Զ�̸�λ
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("3")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "status","Accepted");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

//[3,"b712ecae-6457-4d4f-89b8-93848210555f",{"status":"Accepted"}]
char *ocppProtocl::ChangeAvailabilityReq()//�豸��Ӧƽ̨�·��ı��豸����״̬ 
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("b712ecae")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "status","Accepted");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

/*
[3,"cf8858c0-87d6-459c-acfb-073e97a8cfd1",{"configurationKey":
  [{"key":"LocalAuthListEnabled","readonly":false,"value":"true"},
   {"key":"GetConfigurationMaxKeys","readonly":true,"value":"512"},
   {"key":"MeterValuesAlignedDataMaxLength","readonly":true,"value":"1000"},
   {"key":"MaxEnergyOnInvalidId","readonly":false,"value":"30000"},
   {"key":"WebSocketPingInterval","readonly":false,"value":"10"},
   {"key":"ConnectorPhaseRotationMaxLength","readonly":true,"value":"10"},
   {"key":"ChargingScheduleAllowedChargingRateUnit","readonly":true,"value":"Current"},
   {"key":"LightIntensity","readonly":false,"value":"50"},
   {"key":"ChargingScheduleMaxPeriods","readonly":true,"value":"10"},
   {"key":"AuthorizationCacheEnabled","readonly":false,"value":"true"},
   {"key":"StopTxnAlignedData","readonly":false,"value":"Energy.Active.Import.Register"},
   {"key":"MeterValuesSampledDataMaxLength","readonly":true,"value":"1000"},
   {"key":"LocalPreAuthorize","readonly":false,"value":"true"},
   {"key":"MeterValueSampleInterval","readonly":false,"value":"10"},
   {"key":"HeartbeatInterval","readonly":false,"value":"30"},
   {"key":"NumberOfConnectors","readonly":true,"value":"2"}]}]

*/
char *ocppProtocl::GetConfigurationReq()//�豸��Ӧƽ̨�·���ȡ�豸������Ϣ
{
   cJSON  *pJsonArry,*pJsonsub,*pJsonSub_0,*pJsonSub_1,*pJsonSub_2; 
   cJSON  *pJsonSub_3_0,*pJsonSub_3_1,*pJsonSub_3_2,*pJsonSub_3_3,*pJsonSub_3_4,*pJsonSub_3_5,*pJsonSub_3_6 ;
    
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1281962987467857920")); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   
   cJSON_AddItemToObject(pJsonsub,"configurationKey",pJsonSub_0=cJSON_CreateArray()); 
   
   cJSON_AddItemToArray(pJsonSub_0,pJsonSub_1=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   cJSON_AddStringToObject(pJsonSub_1, "key","HeartbeatInterval");
   cJSON_AddTrueToObject(pJsonSub_1, "readonly");
   cJSON_AddStringToObject(pJsonSub_1, "value","20");
   
   cJSON_AddItemToArray(pJsonSub_0,pJsonSub_2=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   cJSON_AddStringToObject(pJsonSub_2, "key","NumberOfConnectors");
   cJSON_AddTrueToObject(pJsonSub_2, "readonly");
   cJSON_AddStringToObject(pJsonSub_2, "value","2");
   
   cJSON_AddItemToArray(pJsonSub_0,pJsonSub_2=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   cJSON_AddStringToObject(pJsonSub_2, "key"," SupportedFileTransferProtocols");
   cJSON_AddFalseToObject(pJsonSub_2, "readonly");
   cJSON_AddStringToObject(pJsonSub_2, "value","FTP,HTTP");                                                                                        
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   //printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   
   cJSON_Delete(pJsonArry);		
	return 	pJson;
} 

//[3,"f950b518-3167-45e7-9f2a-16dbd5e7189e",{"status":"NotSupported"}]
char *ocppProtocl::ChangeConfigurationReq()//�豸��Ӧƽ̨�·�����豸������Ϣ
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("f950b518")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddStringToObject(pJsonsub, "status","NotSupported");  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

//[3,"1ad56b8b-e596-4218-bfd4-e8c7839a82fd",{"listVersion":0}]
char *ocppProtocl::GetLocalListVersionReq()//�豸��Ӧƽ̨�·���ȡ���ذ������汾
{
   cJSON  *pJsonArry,*pJsonsub;
   
   pJsonArry=cJSON_CreateArray();   /*��������*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateInt(3,1)); /* ���������������ӶԶ���*/
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("1ad56b8b")); /* ���������������ӶԶ���*/   
   //cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateString("RemoteStartTransaction")); /* ���������������ӶԶ���*/   
   cJSON_AddItemToArray(pJsonArry,pJsonsub=cJSON_CreateObject()); /* ���������������ӶԶ���*/
   //cJSON_AddNumberToObject(pJsonsub, "status",1);
   cJSON_AddNumberToObject(pJsonsub, "listVersion",0);  
                                                                                             
   char * pJson = cJSON_PrintUnformatted(pJsonArry);
   printf("%s",pJson);
   if(NULL == pJson)
   {
      cJSON_Delete(pJsonArry);
      return NULL;
   }
   cJSON_Delete(pJsonArry);		
   return 	pJson;
}

//ƽ̨�·���Ϣ�崦����
//{"currentTime":"2020-07-16T02:13:54Z","interval":60,"status":"Accepted"} 
//T��ʾ�ָ�����Z��ʾ����UTC��UTC�������׼ʱ�䣬�ڱ�׼ʱ���ϼ���8Сʱ����������ʱ�䣬Ҳ���Ǳ���ʱ�� 
int ocppProtocl::ParseBootNotificationReq(char *strJson)
{
    cJSON * pJson,*pSub ;
    int iCount=0;
  
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
        return -2;
    }

    char *time = cJSON_GetObjectItem(pJson, "currentTime")->valuestring; //ͬ��ʱ�� 
    
    int heartInterval = cJSON_GetObjectItem(pJson, "interval")->valueint; //������� 
    ocpp_Instance->setOcppMsgPeriod((char *)"Heartbeat",heartInterval); 
    char *status = cJSON_GetObjectItem(pJson, "status")->valuestring;  
    
    //if(memcmp(status,"accept",strlen("accept")))
    if(strcmp(status,"Accepted") == 0)
    {
    	ocpp_Instance->BootNotifiStatus = true;
		ocpp_Instance->setOcppMsgIsSend((char *)"BootNotification",false);
		ocpp_Instance->setOcppMsgIsSend((char *)"Heartbeat",true);
    	printf("ע��ɹ�"); 
	}
	else if(strcmp(status,"Pending") == 0)
	{

	}
	else if(strcmp(status,"Rejected") == 0)
	{

	}
    printf(" ��������= %s %d %s\n",time,heartInterval,status);
	
	std::string recvdatatmp;
	recvdatatmp = HeartbeatReq();//�յ�ע����������һ������
	ocpp_Instance->makebuffToQueue(recvdatatmp);
	return 0;
}

//{"currentTime":"2020-07-16T02:18:51Z"}
int ocppProtocl::ParseHeartbeatReq(char *strJson)
{
    cJSON * pJson,*pSub ;
    int iCount=0;
  
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
      return -2;
    }
    
    char *time = cJSON_GetObjectItem(pJson, "currentTime")->valuestring; //ͬ��ʱ��
	//add ҵ�����߼�
    ocpp_Instance->HeartbeatTimeOutCnt = 0;
    ocpp_Instance->HeartStatus = true;
	
	return 0;
}

//{"connectorId":1,"idTag":"3"}
int ocppProtocl::ParseRemoteStartTransactionReq (char *strJson)
{
    cJSON * pJson,*pSub ;
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
      return -2;
    }
    
    int connectorId = cJSON_GetObjectItem(pJson, "connectorId")->valueint;
    char *idTag = cJSON_GetObjectItem(pJson, "idTag")->valuestring;

    //add ҵ�����߼�
    return 0;	
}

//{"transactionId":42}
int ocppProtocl::ParseRemoteStopTransactionReq (char *strJson)
{
	cJSON * pJson,*pSub ;
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
      return -2;
    }
    
    int transactionId = cJSON_GetObjectItem(pJson, "transactionId")->valueint;
    
	//add ҵ�����߼�
	return 0;
} 
//{"idTagInfo":{"status":"Accepted"}}
int ocppProtocl::ParseAuthorizeReq (char *strJson)
{
    cJSON * pJson,*pSub ;
    int iCount=0;
  
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
      return -2;
    }
    
    pSub = cJSON_GetObjectItem(pJson, "idTagInfo");	
	char *status = cJSON_GetObjectItem(pSub, "status")->valuestring;	  
    printf("��������=%s\n",status);
	
	
	//add ҵ�����߼�
	
	
	return 0;
}

//{"idTagInfo":{"status":"Accepted"},"transactionId":879}
int ocppProtocl::ParseStartTransactionReq(char *strJson)
{
	cJSON * pJson,*pSub ;
    int iCount=0;
  
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
        return -2;
    }
    
    pSub = cJSON_GetObjectItem(pJson, "idTagInfo");	
	char *status = cJSON_GetObjectItem(pSub, "status")->valuestring;
	int transactionId = cJSON_GetObjectItem(pJson, "transactionId")->valueint;	  
    printf("��������=%s %d\n",status,transactionId);
	
	return 0;
    
}

//{}������ 
int ocppProtocl::ParseMeterValuesReq(char *strJson)
{
   printf("�յ�MeterValuesResӦ��");
   return 0;   
}

//{}
int ocppProtocl::ParseStopTransactionReq(char *strJson)
{
	printf("�յ�StopTransactionResӦ��");
	return 0;
}

//{}
int ocppProtocl::ParseStatusNotificationReq(char *strJson)
{
	printf("�յ�StatusNotificationResӦ��");
	return 0;
}

//{"type":"Hard"}
int ocppProtocl::ParseResetReq(char *strJson)//ƽ̨�·�Զ�̸�λ 
{
	cJSON * pJson,*pSub ;
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
      return -2;
    }
    
    char * type = cJSON_GetObjectItem(pJson, "type")->valuestring;
    printf("Reset:%s\n",type);
	//add ҵ�����߼�
	return 0;
}

//{"retrieveDate":"2020-07-21T08:08:28Z","location":"http://www.baidu.com/Firmware/OCPPnoTouch.zip"}
//The FTP URL is of format: ftp://user:password@host:port/path 
int ocppProtocl::ParseUpdateFirmwareReq(char *strJson)//ƽ̨�·�Զ�������� 
{
	cJSON * pJson,*pSub ;
	
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
        return -2;
    }
    
    char * location = cJSON_GetObjectItem(pJson, "location")->valuestring;
    
    pSub = cJSON_GetObjectItem(pJson, "retrieveDate");
    if(pSub == NULL)
    {
    	 printf("no retrieveDate\n");
	}
	else
	{
		char * retrieveDate = cJSON_GetObjectItem(pJson, "retrieveDate")->valuestring;
		printf("retrieveDate:%s,location:%s\n",retrieveDate,location);
	} 

	char *transferType = ocpp_Instance->ocppUilsChange->getStrchr(location,':');
	char *url = strchr(location,':')+1;
	if(strcmp(transferType,"ftp") == 0)//URL: ftp://user:password@host:port/path
	{
	   printf("ftp ����\n\n");
	   
	   //add ҵ�����߼�
	}
	else if(strcmp(transferType,"http") == 0)
	{
	    
        ocpp_Instance->httpClientobject->parseCjsonURL(url);
        ocpp_Instance->httpClientobject->httpPostFile();

        printf("http ����\n\n");
        //add ҵ�����߼�
	}

	delete transferType;
	
	return 0;
}

int ocppProtocl::ParseFirmwareStatusNotificationReq(char *strJson)//ƽ̨��Ӧ�豸�����ļ�״̬
{
    printf("�յ�FirmwareStatusNotificationResӦ��");
    return 0;
}

//{"startTime":"2020-07-21T08:06:26Z","stopTime":"2020-07-21T08:06:26Z","location":"http://www.baidu.com/rizhi/1285486278560976896"}
int ocppProtocl::ParseGetDiagnosticsReq(char *strJson)///ƽ̨�·�Զ����־���� 
{
	cJSON * pJson,*pSub ;
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
      return -2;
    }
    char * location = cJSON_GetObjectItem(pJson, "location")->valuestring;
    pSub = cJSON_GetObjectItem(pJson, "startTime");
    if(pSub == NULL)
    {
    	printf("no startTime");
	}	
    pSub = cJSON_GetObjectItem(pJson, "stopTime");
    if(pSub == NULL)
    {
    	printf("no stopTime");
	}
	else
    {
	    char * stopTime = pSub->valuestring;
		printf("stopTime:%s,location:%s",stopTime,location);
    } 
    return 0;	  
}

int ocppProtocl::ParseDiagnosticsStatusNotificationReq(char *strJson)//ƽ̨��Ӧ���豸�ϴ��ļ�״̬
{
	printf("�յ�DiagnosticsStatusNotificationres");
    return 0;
}

//[2,"b712ecae-6457-4d4f-89b8-93848210555f","ChangeAvailability",{"connectorId":1,"type":"Operative"}]
int ocppProtocl::ParseChangeAvailabilityReq(char *strJson)//ƽ̨�·��ı��豸����״̬
{
	cJSON * pJson,*pSub ;
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
        return -2;
    }

    int connectorId = cJSON_GetObjectItem(pJson, "connectorId")->valueint;	
    char *type = cJSON_GetObjectItem(pJson, "type")->valuestring; //��Inoperative�� "Operative"
	printf("ChangeAvailability:%d %s\n",connectorId,type);
	
	
	return 0;
}
 
//[2,"cf8858c0-87d6-459c-acfb-073e97a8cfd1","GetConfiguration",{}]
int ocppProtocl::ParseGetConfigurationReq(char *strJson)//ƽ̨�·���ȡ�豸������Ϣ 
{
	printf("GetConfiguration\n");
	return 0;
}

//{"key":"AuthorizationKey","value":"EVKv00OYlFuMhmrAuZQS0XGY8g5VjWPsZGNTcdf1"}
int ocppProtocl::ParseChangeConfigurationReq(char *strJson)//ƽ̨�·�����豸������Ϣ
{
	cJSON * pJson,*pSub ;
    if(NULL == strJson)
    {
        return -1;
    }
  
    pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
    if(NULL == pJson)
    {
        return -2;
    }

    char *key = cJSON_GetObjectItem(pJson, "key")->valuestring;	
    char *value = cJSON_GetObjectItem(pJson, "value")->valuestring; 
	printf("ChangeConfiguration:%s %s\n",key,value);
	return 0;
}

//[2,"1ad56b8b-e596-4218-bfd4-e8c7839a82fd","GetLocalListVersion",{}]
int ocppProtocl::ParseGetLocalListVersionReq(char *strJson)//ƽ̨�·���ȡ���ذ������汾
{
	printf("GetLocalListVersion\n");
	return 0;
}

int ocppProtocl::ParseOcppProtocl(char *pMsgSque,char *pMsgId,char *pMsgBody)
{
	
	int OcppArryid = -1;
	std::string recvdatatmp;
	if(pMsgId == NULL) //��Ӧ��Ϣ
	{
		OcppArryid = ocpp_Instance->getOcppMsgArryId(NULL,pMsgSque);
		printf("������Ӧ��Ϣ arrayId= %d, MsgSque=%s, msgbody=%s\n",OcppArryid,pMsgSque,pMsgBody); 
		if(OcppArryid < 0)
		{
			return -1;
		}
		
	    OcppProtoclReqArry[OcppArryid].funcProtoclReq(pMsgBody);	
		if(OcppProtoclReqArry[OcppArryid].funcProtoclRes == NULL)
		{
			printf("����Ӧ����Ҫ����\n\n"); 
			return 0; 
		}
        return 1;
	}
	else //������Ϣ
	{
		OcppArryid = ocpp_Instance->getOcppMsgArryId(pMsgId,NULL);
		printf("����������Ϣ arrayId= %d,MsgSque=%s, MsgId=%s, msgbody=%s",OcppArryid,pMsgSque,pMsgId,pMsgBody); 
		if(OcppArryid < 0)
		{
			return -1;
		}

		ocpp_Instance->setOcppMsgSque(pMsgId,pMsgSque);
		OcppProtoclReqArry[OcppArryid].funcProtoclReq(pMsgBody);	
		if(OcppProtoclReqArry[OcppArryid].funcProtoclRes == NULL)
		{
			printf("��������Ҫ����\n\n"); 
			return 0; 
		}
		else
		{
		   recvdatatmp = OcppProtoclReqArry[OcppArryid].funcProtoclRes();	
    	   ocpp_Instance->makebuffToQueue(recvdatatmp);
		}
		return 1;	
	}
} 

//ƽ̨�����·�: [2,"3","RemoteStartTransaction",{"connectorId":1,"idTag":"3"}]
//ƽ̨������Ӧ: [3,"1281962846321139712",{"idTagInfo":{"status":"Accepted"}}] 
int ocppProtocl::ParseWebsocketJsonStr (char *strJson)
{
  cJSON * pJson,*pSub,*pSub1,*pSub2,*pSub3 ;
  int iCount=0;
  
  if(NULL == strJson)
  {
        return -1;
  }
  
  pJson = cJSON_Parse(strJson);  /* ���� json ���� pJson*/
  if(NULL == pJson)
  {
    return -2;
  }
  
  //ƽ̨�����·�: [2,"3","RemoteStartTransaction",{"connectorId":1,"idTag":"3"}]
  //ƽ̨������Ӧ: [3,"1281962846321139712",{"idTagInfo":{"status":"Accepted"}}]   
  printf("��������=%s\n",cJSON_PrintUnformatted(pJson));
  iCount = cJSON_GetArraySize(pJson); /*��ȡ���鳤��*/
  //printf("���鳤��=%d\n",iCount);
  if(iCount < 2)
  {
  	return -3;
  }
  pSub = cJSON_GetArrayItem(pJson,0);  
  //printf("�������������=%lld ���� %d\n",pSub->valueint,pSub->type);
  if(pSub->type != cJSON_Int)
  {
  	 return -4;	
  }
  pSub1 = cJSON_GetArrayItem(pJson,1); //��Ϣ���к� 
  //printf("��Ϣ���к�=%s ���� %d\n",pSub1->valuestring,pSub1->type);
  pSub2 = cJSON_GetArrayItem(pJson,2); //����:��Ϣid  ��Ӧ:��Ϣ��  
  
  if(pSub->valueint == REQUEST) //���� 
  {
  
    pSub3 = cJSON_GetArrayItem(pJson,3); //��Ϣ�� 
	char *pSubjson = cJSON_PrintUnformatted(pSub3); 
    //printf("������ϢId=%s,��Ϣ������=%s ���� %d\n",pSub2->valuestring,pSubjson,pSub3->type);
    ParseOcppProtocl(pSub1->valuestring,pSub2->valuestring,pSubjson);
  }
  else if(pSub->valueint == RESPONSE) //��Ӧ 
  {
  	char *pSubjson = cJSON_PrintUnformatted(pSub2);
	//printf("��Ӧ����Ϣ������=%s ���� %d\n",pSubjson,pSub2->type); 
    ParseOcppProtocl(pSub1->valuestring,NULL,pSubjson);
  }

  cJSON_Delete(pJson);
  return 0;
}

/*******************************************************************************************/
/**************************************ҵ�����̴���*****************************************/ 
void ocppProtocl::enableHeartTimeoutFun()
{	
	static int heartBeatid = 0,heartPeroid = 0; //����id��������λ�� 
	static int Cnt = 0;
   	char *heartBeat = (char *)"Heartbeat";
   	
	if(BootNotifiStatus == false)
	{
	    Cnt =0;
	    return;
	}
	
	if(!heartBeatid)
	{
		heartBeatid = getOcppMsgArryId(heartBeat,NULL);
		if(heartBeatid < 0)
		{
		   printf("heartBeatû�п�ִ����\n");
		   Cnt = 0;
		   HeartbeatTimeOutCnt = 0;	
		}
		else
		{
			heartPeroid = OcppProtoclReqArry[heartBeatid].period;
			printf("heartPeroid = %d\n",heartPeroid);
		}
	}
	Cnt++;
    if(heartBeatid > 0 && Cnt*10 > heartPeroid)
	{
	   HeartbeatTimeOutCnt++;
	   printf("HeartbeatTimeOutCnt = %d\n",HeartbeatTimeOutCnt);
	   Cnt = 0;
	}
	
	if(HeartbeatTimeOutCnt > HEART_TIMEOUT_CNT)
	{
		 printf("heartBeat_timeout\n");
	}
}

void ocppProtocl::enableHeartBeatFun()//ʹ���������� ����
{

}

void ocppProtocl::enableMeterValuesFun() //ʹ�ܳ�������Ϣ���� ���� 
{
	
}

void ocppProtocl::enableStatusNotifFun() //ʹ���豸״̬�仯���� ���� 
{
    int msgId = 0;
	char *StatusNotif = (char *)"StatusNotification";
    static int CharerPointStatusBak ; 
    /*if(ocpp_Instance->HeartStatus == false)
    {
        return;
    }*/
	msgId = getOcppMsgArryId(StatusNotif,NULL);
	if(msgId < 0)
	{
	   printf("StatusNotificationû�п�ִ����\n");
	}
	
	if(ChargerPointStatus != CharerPointStatusBak) 
	{
	   OcppProtoclReqArry[msgId].isSend = true;	
	   CharerPointStatusBak = ChargerPointStatus;
	}
}

void ocppProtocl::enablePeriodFun()//ʹ�ܷ��ͺ����˺���100ms����ִ�� 
{
	//enableHeartBeatFun();
	enableHeartTimeoutFun();
	//enableMeterValuesFun();
	//enableStatusNotifFun();
}

void ocppProtocl::OcppProtoclFunRun() //�˺����������� T=100ms
{
   	int arraySize = getOcppProtoclReqArrySize();
    int id = 0;
	static int Cnt = 0;
	std::string recvdatatmp;
   	if(arraySize < 1)
   	{
   		printf("error,û�п�ִ��ѡ��\n");
		return; 
	} 

	Cnt++;
    if(ocpp_Instance->BootNotifiStatus == false)
    {
      if(Cnt%(OcppProtoclReqArry[0].period*10) == 0)
      {
    	  recvdatatmp = OcppProtoclReqArry[0].funcProtoclRes();
    	  makebuffToQueue(recvdatatmp);
      }
    }
	else
	{
		for(id = 1;id < arraySize; id++)
		{
		  
		   if(OcppProtoclReqArry[id].period > 0)//���ں���
		   {
			   if(OcppProtoclReqArry[id].isSend == true  && 
				  Cnt%(OcppProtoclReqArry[id].period*10) == 0)	
			   {
			       recvdatatmp = OcppProtoclReqArry[id].funcProtoclRes();
				   makebuffToQueue(recvdatatmp); 
			   }

		   }
		   else //������
		   {
			   if(OcppProtoclReqArry[id].isSend == true )
			   {
                    recvdatatmp = OcppProtoclReqArry[id].funcProtoclRes();
                    OcppProtoclReqArry[id].isSend = false;
                    makebuffToQueue(recvdatatmp);
			   }
		   }
		} 
	}
}

void ocppProtocl::taskRun100ms()
{
  enablePeriodFun();	
  OcppProtoclFunRun();
}

void ocppProtocl::ocppProtoclInit()
{
	startProtoclThread();
}

void *ocppProtocl::run(void *arg)
{ 
    int xxx = 0; 
    char t[10];
    std::string recvdatatmp;
	
    ocppMsgQueue::OCPPMSGQueueNode* recvdata = NULL;
    while(1)
    {
        taskRun100ms();
		recvdata = queue->DeQueue(queue->ocpp_msg_recv_queue);
        if(recvdata != NULL)
        {
            ParseWebsocketJsonStr((char *)recvdata->data);
            delete recvdata;
        }
        else
        {
            //printf("Sorry recv NULL \n");
        }
        mSleep(100);
    }
}

int ocppProtocl::makebuffToQueue(std::string &message)
{
    OCPPMSGQueueNode *queueRecv = new OCPPMSGQueueNode;
    //printf("message = %s  message.length() = %d  \n",message.c_str(),message.length()+1);
    queueRecv->data = malloc(message.length()+1); 
    strcpy((char *)queueRecv->data,message.c_str());//message.c_str());
    queueRecv->size = message.length()+1;
    queueRecv->index =1;
    
    queue->EnQueue(queue->ocpp_msg_send_queue,queueRecv);
}


int ocppProtocl::startProtoclThread()
{
    int ret=0;
    ret = pthread_create(&pidTaskRun, NULL, (void * ( *)(void *))&ocppProtocl::run, (void *)this);
    if(ret)
    {
        printf("create pthread error!\n");
        return -1; 
    }
        printf("create pthread success!\n");
    return 0;
}

}
