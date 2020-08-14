#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__
namespace ocppMsgQueue{
typedef enum {
    OCPPMSGRecvQueue,
    OCPPMSGSendQueue
} OCPPMSGQueueType;

typedef struct OCPPMSGQueueNode {
    void    *data;
    int     size;  // data size
    long    index;
    struct  OCPPMSGQueueNode *next;
} OCPPMSGQueueNode;

typedef struct OCPPMSGQueue {
    int size;
    OCPPMSGQueueType type;
    OCPPMSGQueueNode *front;
    OCPPMSGQueueNode *rear;
} OCPPMSGQueue;

class OCPPMSGQueueProcess {
    
private:

	// ���俽������͸�ֵ�����Ϊ˽�к���, ��ֹ�ⲿ�����͸�ֵ
    OCPPMSGQueueProcess(const OCPPMSGQueueProcess &signal);
    const OCPPMSGQueueProcess &operator=(const OCPPMSGQueueProcess &signal);
	//�����캯���������������ĳ�˽��
    OCPPMSGQueueProcess(){};
    ~OCPPMSGQueueProcess(){};
	
	//���в�����
    pthread_mutex_t send_queue_mutex;
    pthread_mutex_t recv_queue_mutex;

public:

    static pthread_mutex_t singletonMtx;//������
    static OCPPMSGQueueProcess* GetInstance();
	
    //��Ϣ����
    OCPPMSGQueue *ocpp_msg_send_queue;
    OCPPMSGQueue *ocpp_msg_recv_queue;
 
    // ��Ϣ���� Operation
	void CreatQueue(int OCPPMSGQueueSize);
    void InitQueue(OCPPMSGQueue *queue, OCPPMSGQueueType type);
    int GetQueueSize(OCPPMSGQueue *queue);
    void EnQueue(OCPPMSGQueue *queue,OCPPMSGQueueNode *node);
    OCPPMSGQueueNode *DeQueue(OCPPMSGQueue *queue);
    void ClearOCPPMSGQueue(OCPPMSGQueue *queue);
    void FreeNode(OCPPMSGQueueNode* node);
    void ResetFreeQueue(OCPPMSGQueue *workQueue, OCPPMSGQueue *FreeQueue);
    
};
}
#endif
