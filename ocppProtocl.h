#ifndef __OCPP_PROTOCL_H__
#define __OCPP_PROTOCL_H__
#include "ocppclient.h"
#include "msgQueue.h"
#include "ocppUtils.h"
#include "httpClient.h"

#define bool int

using namespace ocppMsgQueue;
using namespace httpGetPost;

namespace ocppProtoclHand{
          
enum{SHELL_INFO_NUM_MAX=16};

typedef struct OcppProtoclReq
{
	int id;
	char *pMsgSque; //��Ϣ���к� 
	char *pMsgId;//��Ϣ������ 
	bool isRequst; //����Ϣ�Ƿ���ҪӦ��----���Զ������. 
	bool isSend; //����Ϣ�Ƿ���Ҫ���Ϸ���--���ע��.��翪ʼ��Ϣ.״̬��Ϣ(״̬�л������ϴ�).ֹͣ�����Ϣ 
	unsigned char period; //�豸������Ϣ������ ---��Գ�������Ϣ.������Ϣ.����Ȩ 
	int (*funcProtoclReq)(char *pJson); //ƽ̨�·�������      
	char* (*funcProtoclRes)(); //�豸�ϴ������� 
}OcppProtoclReq;

class ocppProtocl
{
public:
    /*typedef struct SOuterHandles
    {
        CChargeCore        *        pChargeCore_;
        CLogicalStorageInterface  * pStorageInterface_;
    } SOuterHandles;        //���õ����ⲿ���handles���ڴ���ʱ���롣��Աһ��Ϊָ�롣

    typedef struct SConstParams
    {
        uint8       chargePortCount_;
    } SConstParams;         //��ĳ������ò�����ʵ�������ڼ䲻�ɸı䣬�ڴ���ʱ����

    typedef struct SCreateParams
    {
        SOuterHandles   outerHandles_;
        SConstParams    constParams_;
    } SCreateParams;        //��Ĵ�������������ʱ�������캯��
    */

    ocppProtocl();//const SCreateParams & rCreateParams);
    ~ocppProtocl();
    
    int startProtoclThread();
    void taskRun100ms();
    void ocppProtoclInit();
    static ocppProtocl *getInstance();
    int makebuffToQueue(std::string & message);

private:
	//��Ա����
	void mSleep(int ms);
    int getOcppProtoclReqArrySize();
    int getOcppMsgArryId(char *pMsgId,char *pMsgSque);
    int setOcppMsgSque(char *pMsgId,char *pMsgSque);
    char *getOcppMsgSque(char *pMsgId);
    int setOcppMsgIsRequst(char *pMsgId,bool isRequst);
    int setOcppMsgIsSend(char *pMsgId,bool isSend);
    int setOcppMsgPeriod(char *pMsgId,bool period);
    int getOcppMsgPeriod(int Id);

    void enableHeartTimeoutFun();
    void enableHeartBeatFun();
    void enableMeterValuesFun();
    void enableStatusNotifFun();
    void enablePeriodFun();

    //�豸�ϴ� 
    static char *BootNotificationReq();//�豸����ע����Ϣ 
    static char *HeartbeatReq();//�豸����������Ϣ
    static char *RemoteStartTransactionReq();//�豸��ӦԶ�̿�����Ϣ 
    static char *RemoteStopTransactionReq();//�豸��ӦԶ��ͣ����Ϣ
    static char *AuthorizeReq();//�豸����У����Ϣ 
    static char *StartTransactionReq();//�豸����ʼ�����Ϣ
    static char *MeterValuesReq();//�豸�����������Ϣ
    static char *StopTransactionReq();//�豸����ֹͣ�����Ϣ
    static char *StatusNotificationReq();//�豸�����豸״̬��Ϣ
    static char *UpdateFirmwareReq();//�豸��Ӧƽ̨���� 
    static char *FirmwareStatusNotificationReq();//�豸�ϴ� �����ļ�״̬
    static char *GetDiagnosticsReq();//�豸��Ӧƽ̨��־���� 
    static char *DiagnosticsStatusNotificationReq();//�豸�ϴ� �ϴ��ļ�״̬
    static char *ResetReq();//�豸��Ӧƽ̨�·�Զ�̸�λ
    static char *ChangeAvailabilityReq();//�豸��Ӧƽ̨�·��ı��豸����״̬ 
    static char *GetConfigurationReq();//�豸��Ӧƽ̨�·���ȡ�豸������Ϣ 
    static char *ChangeConfigurationReq();//�豸��Ӧƽ̨�·�����豸������Ϣ
    static char *GetLocalListVersionReq();//�豸��Ӧƽ̨�·���ȡ���ذ������汾

    //ƽ̨�·� 
    static int ParseBootNotificationReq(char *strJson); //ƽ̨��Ӧע��
    static int ParseHeartbeatReq(char *strJson); //ƽ̨��Ӧ���� 
    static int ParseRemoteStartTransactionReq (char *strJson);//Զ������ 
    static int ParseRemoteStopTransactionReq (char *strJson);//Զ��ͣ�� 
    static int ParseAuthorizeReq (char *strJson);//ƽ̨��Ӧ��Ȩ
    static int ParseStartTransactionReq(char *strJson);//ƽ̨��Ӧ��翪ʼ��Ϣ
    static int ParseMeterValuesReq(char *strJson);//ƽ̨��Ӧ��������Ϣ
    static int ParseStopTransactionReq(char *strJson);//ƽ̨��Ӧ��������Ϣ 
    static int ParseStatusNotificationReq(char *strJson);//ƽ̨��Ӧ״̬��Ϣ
    static int ParseResetReq(char *strJson);//ƽ̨�·�Զ�̸�λ 
    static int ParseUpdateFirmwareReq(char *strJson);//ƽ̨�·�Զ�������� 
    static int ParseFirmwareStatusNotificationReq(char *strJson);//ƽ̨��Ӧ�豸�����ļ�״̬
    static int ParseGetDiagnosticsReq(char *strJson);//ƽ̨�·�Զ����־���� 
    static int ParseDiagnosticsStatusNotificationReq(char *strJson);//ƽ̨��Ӧ���豸�ϴ��ļ�״̬
    static int ParseChangeAvailabilityReq(char *strJson);//ƽ̨�·��ı��豸����״̬ 
    static int ParseGetConfigurationReq(char *strJson);//ƽ̨�·���ȡ�豸������Ϣ 
    static int ParseChangeConfigurationReq(char *strJson);//ƽ̨�·�����豸������Ϣ
    static int ParseGetLocalListVersionReq(char *strJson);//ƽ̨�·���ȡ���ذ������汾
    static int ParseOcppProtocl(char *pMsgSque,char *pMsgId,char *pMsgBody);
    static int ParseWebsocketJsonStr (char *strJson);

    void OcppProtoclFunRun();
    int OcppProtoclReqArryInit();

    void *run(void *arg);

    OCPPMSGQueueProcess *queue;
    //��Ա����
    static OcppProtoclReq OcppProtoclReqArry[SHELL_INFO_NUM_MAX];

    static ocppProtocl *ocpp_Instance;
    ocppUils *ocppUilsChange;
    httpClient *httpClientobject;
	bool  BootNotifiStatus; //false:δע��  true��ע��ɹ�
	bool  HeartStatus; //false:δ��  true����ͨ
	int   HeartbeatInterv;     //Ĭ��������� 10s
	int   HeartbeatTimeOutCnt;  //������ʱ���� 0
	int   ChargerPointStatus; //���׮�豸״̬ 0:����  1�����  2��������  3������ 
    pthread_t pidTaskRun;


    //const SOuterHandles     outerHandles_;
    //const SConstParams      constParams_;
};	
}



#endif
