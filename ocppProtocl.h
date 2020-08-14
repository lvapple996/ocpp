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
	char *pMsgSque; //消息序列号 
	char *pMsgId;//消息命令字 
	bool isRequst; //该消息是否需要应答----针对远程启动. 
	bool isSend; //该消息是否需要马上发送--针对注册.充电开始消息.状态消息(状态切换马上上传).停止充电消息 
	unsigned char period; //设备请求消息的周期 ---针对充电过程消息.心跳消息.卡鉴权 
	int (*funcProtoclReq)(char *pJson); //平台下发处理函数      
	char* (*funcProtoclRes)(); //设备上传处理函数 
}OcppProtoclReq;

class ocppProtocl
{
public:
    /*typedef struct SOuterHandles
    {
        CChargeCore        *        pChargeCore_;
        CLogicalStorageInterface  * pStorageInterface_;
    } SOuterHandles;        //类用到的外部句柄handles，在创建时传入。成员一般为指针。

    typedef struct SConstParams
    {
        uint8       chargePortCount_;
    } SConstParams;         //类的常量配置参数，实例存在期间不可改变，在创建时传入

    typedef struct SCreateParams
    {
        SOuterHandles   outerHandles_;
        SConstParams    constParams_;
    } SCreateParams;        //类的创建参数，创建时传给构造函数
    */

    ocppProtocl();//const SCreateParams & rCreateParams);
    ~ocppProtocl();
    
    int startProtoclThread();
    void taskRun100ms();
    void ocppProtoclInit();
    static ocppProtocl *getInstance();
    int makebuffToQueue(std::string & message);

private:
	//成员函数
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

    //设备上传 
    static char *BootNotificationReq();//设备请求注册消息 
    static char *HeartbeatReq();//设备请求心跳消息
    static char *RemoteStartTransactionReq();//设备响应远程开机消息 
    static char *RemoteStopTransactionReq();//设备响应远程停机消息
    static char *AuthorizeReq();//设备请求卡校验消息 
    static char *StartTransactionReq();//设备请求开始充电消息
    static char *MeterValuesReq();//设备请求充电过程消息
    static char *StopTransactionReq();//设备请求停止充电消息
    static char *StatusNotificationReq();//设备请求设备状态消息
    static char *UpdateFirmwareReq();//设备响应平台升级 
    static char *FirmwareStatusNotificationReq();//设备上传 下载文件状态
    static char *GetDiagnosticsReq();//设备响应平台日志调用 
    static char *DiagnosticsStatusNotificationReq();//设备上传 上传文件状态
    static char *ResetReq();//设备响应平台下发远程复位
    static char *ChangeAvailabilityReq();//设备响应平台下发改变设备服务状态 
    static char *GetConfigurationReq();//设备响应平台下发获取设备配置信息 
    static char *ChangeConfigurationReq();//设备响应平台下发变更设备配置信息
    static char *GetLocalListVersionReq();//设备响应平台下发获取本地白名单版本

    //平台下发 
    static int ParseBootNotificationReq(char *strJson); //平台响应注册
    static int ParseHeartbeatReq(char *strJson); //平台响应心跳 
    static int ParseRemoteStartTransactionReq (char *strJson);//远程启动 
    static int ParseRemoteStopTransactionReq (char *strJson);//远程停机 
    static int ParseAuthorizeReq (char *strJson);//平台响应鉴权
    static int ParseStartTransactionReq(char *strJson);//平台响应充电开始消息
    static int ParseMeterValuesReq(char *strJson);//平台响应充电过程消息
    static int ParseStopTransactionReq(char *strJson);//平台响应充电结束消息 
    static int ParseStatusNotificationReq(char *strJson);//平台响应状态消息
    static int ParseResetReq(char *strJson);//平台下发远程复位 
    static int ParseUpdateFirmwareReq(char *strJson);//平台下发远程设升级 
    static int ParseFirmwareStatusNotificationReq(char *strJson);//平台响应设备下载文件状态
    static int ParseGetDiagnosticsReq(char *strJson);//平台下发远程日志调用 
    static int ParseDiagnosticsStatusNotificationReq(char *strJson);//平台响应设设备上传文件状态
    static int ParseChangeAvailabilityReq(char *strJson);//平台下发改变设备服务状态 
    static int ParseGetConfigurationReq(char *strJson);//平台下发获取设备配置信息 
    static int ParseChangeConfigurationReq(char *strJson);//平台下发变更设备配置信息
    static int ParseGetLocalListVersionReq(char *strJson);//平台下发获取本地白名单版本
    static int ParseOcppProtocl(char *pMsgSque,char *pMsgId,char *pMsgBody);
    static int ParseWebsocketJsonStr (char *strJson);

    void OcppProtoclFunRun();
    int OcppProtoclReqArryInit();

    void *run(void *arg);

    OCPPMSGQueueProcess *queue;
    //成员变量
    static OcppProtoclReq OcppProtoclReqArry[SHELL_INFO_NUM_MAX];

    static ocppProtocl *ocpp_Instance;
    ocppUils *ocppUilsChange;
    httpClient *httpClientobject;
	bool  BootNotifiStatus; //false:未注册  true：注册成功
	bool  HeartStatus; //false:未发  true：已通
	int   HeartbeatInterv;     //默认心跳间隔 10s
	int   HeartbeatTimeOutCnt;  //心跳超时次数 0
	int   ChargerPointStatus; //充电桩设备状态 0:待机  1：充电  2：充电结束  3：故障 
    pthread_t pidTaskRun;


    //const SOuterHandles     outerHandles_;
    //const SConstParams      constParams_;
};	
}



#endif
