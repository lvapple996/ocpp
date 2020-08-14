
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "msgQueue.h"


namespace ocppMsgQueue{
	
pthread_mutex_t OCPPMSGQueueProcess::singletonMtx = PTHREAD_MUTEX_INITIALIZER;
OCPPMSGQueueProcess* OCPPMSGQueueProcess::GetInstance()
{
    pthread_mutex_lock(&singletonMtx);
    static OCPPMSGQueueProcess obj;
    pthread_mutex_unlock(&singletonMtx);
    return &obj;
}

void OCPPMSGQueueProcess::CreatQueue(int OCPPMSGQueueSize) {
    ocpp_msg_send_queue = (OCPPMSGQueue *)malloc(sizeof(struct OCPPMSGQueue));
    ocpp_msg_recv_queue = (OCPPMSGQueue *)malloc(sizeof(struct OCPPMSGQueue));
    
    InitQueue(ocpp_msg_send_queue, OCPPMSGSendQueue);
    InitQueue(ocpp_msg_recv_queue, OCPPMSGRecvQueue);
    
    for (int i = 0; i < OCPPMSGQueueSize; i++) {
        OCPPMSGQueueNode *node = (OCPPMSGQueueNode *)malloc(sizeof(struct OCPPMSGQueueNode));
        node->data = NULL;
        node->size = 0;
        node->index= 0;
        this->EnQueue(ocpp_msg_send_queue, node);
    }
    
    pthread_mutex_init(&send_queue_mutex, NULL);
    pthread_mutex_init(&recv_queue_mutex, NULL);
    
    printf("OCPPMSGQueueProcess Init finish !\n");
}

void OCPPMSGQueueProcess::InitQueue(OCPPMSGQueue *queue, OCPPMSGQueueType type) {
    if (queue != NULL) {
        queue->type  = type;
        queue->size  = 0;
        queue->front = 0;
        queue->rear  = 0;
    }
}

int GetQueueSize(OCPPMSGQueue *queue)
{
    return queue->size;
}

void OCPPMSGQueueProcess::EnQueue(OCPPMSGQueue *queue, OCPPMSGQueueNode *node) {
    if (queue == NULL) {
        printf("OCPPMSGQueueProcess Enqueue : current queue is NULL\n");
        return;
    }

    if (node==NULL) {
        printf("OCPPMSGQueueProcess Enqueue : current node is NULL\n");
        return;
    }
    
    node->next = NULL;
    
    if (OCPPMSGSendQueue == queue->type) {
        pthread_mutex_lock(&send_queue_mutex);
        
        if (queue->front == NULL) {
            queue->front = node;
            queue->rear  = node;
        }else {
            
             // tail in,head out
             queue->rear->next = node;
             queue->rear = node;
            
            
            // head in,head out
            //node->next = queue->front;
            //queue->front = node;
        }
        queue->size += 1;
        printf("OCPPMSGQueueProcess Enqueue :  send queue size=%d\n",queue->size);
        pthread_mutex_unlock(&send_queue_mutex);
    }
    
    if (OCPPMSGRecvQueue == queue->type) {
        pthread_mutex_lock(&recv_queue_mutex);
        //TODO
        static long nodeIndex = 0;
        node->index=(++nodeIndex);
        if (queue->front == NULL) {
            queue->front = node;
            queue->rear  = node;
        }else {
            queue->rear->next   = node;
            queue->rear         = node;
        }
        queue->size += 1;
        printf("OCPPMSGQueueProcess Enqueue : recv queue size=%d\n",queue->size);
        pthread_mutex_unlock(&recv_queue_mutex);
    }
}

OCPPMSGQueueNode* OCPPMSGQueueProcess::DeQueue(OCPPMSGQueue *queue) {
    if (queue == NULL) {
        printf("OCPPMSGQueueProcess DeQueue : current queue is NULL\n");
        return NULL;
    }
    
    const char *type = queue->type == OCPPMSGRecvQueue ? "recv queue" : "send queue";
    pthread_mutex_t *queue_mutex = ((queue->type == OCPPMSGRecvQueue) ? &recv_queue_mutex : &send_queue_mutex);
    OCPPMSGQueueNode *element = NULL;
    
    pthread_mutex_lock(queue_mutex);
    element = queue->front;

    if(element == NULL) {
        pthread_mutex_unlock(queue_mutex);
        //printf("OCPPMSGQueueProcess DeQueue : The node is NULL\n");
        return NULL;
    }
    if(queue->size>1)
    {
        queue->front = queue->front->next;
    }
    else
    {
        queue->front = NULL;
    }
    queue->size -= 1;
    
    pthread_mutex_unlock(queue_mutex);
    
    printf("OCPPMSGQueueProcess DeQueue : %s size=%d\n",type,queue->size);
    return element;
}

void OCPPMSGQueueProcess::ClearOCPPMSGQueue(OCPPMSGQueue *queue) {
    while (queue->size) {
        OCPPMSGQueueNode *node = this->DeQueue(queue);
        this->FreeNode(node);
    }

    printf("%s: Clear OCPPMSGQueueProcess queue\n",__func__);
}

void OCPPMSGQueueProcess::FreeNode(OCPPMSGQueueNode* node) {
    if(node != NULL){
        free(node->data);
        free(node);
    }
}

void OCPPMSGQueueProcess::ResetFreeQueue(OCPPMSGQueue *workQueue, OCPPMSGQueue *freeQueue) {
    if (workQueue == NULL) {
        printf("OCPPMSGQueueProcess ResetFreeQueue : The WorkQueue is NULL\n");
        return;
    }
    
    if (freeQueue == NULL) {
        printf("OCPPMSGQueueProcess ResetFreeQueue : The FreeQueue is NULL\n");
        return;
    }
    
    int workQueueSize = workQueue->size;
    if (workQueueSize > 0) {
        for (int i = 0; i < workQueueSize; i++) {
            OCPPMSGQueueNode *node = DeQueue(workQueue);
            //CFRelease(node->data);
            free(node->data);
            node->data = NULL;
            EnQueue(freeQueue, node);
        }
    }
    printf("OCPPMSGQueueProcess ResetFreeQueue : The work queue size is %d, free queue size is %d\n\n",workQueue->size, freeQueue->size);
}
}

