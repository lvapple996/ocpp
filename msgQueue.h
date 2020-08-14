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

	// 将其拷贝构造和赋值构造成为私有函数, 禁止外部拷贝和赋值
    OCPPMSGQueueProcess(const OCPPMSGQueueProcess &signal);
    const OCPPMSGQueueProcess &operator=(const OCPPMSGQueueProcess &signal);
	//将构造函数和析构函数都改成私有
    OCPPMSGQueueProcess(){};
    ~OCPPMSGQueueProcess(){};
	
	//队列操作锁
    pthread_mutex_t send_queue_mutex;
    pthread_mutex_t recv_queue_mutex;

public:

    static pthread_mutex_t singletonMtx;//单例锁
    static OCPPMSGQueueProcess* GetInstance();
	
    //消息队列
    OCPPMSGQueue *ocpp_msg_send_queue;
    OCPPMSGQueue *ocpp_msg_recv_queue;
 
    // 消息队列 Operation
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
