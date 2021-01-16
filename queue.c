#include "queue.h"
#include <stdbool.h>

static QueueElement *queueElementCreate(void *val){
    QueueElement *newElement = (QueueElement*)malloc(sizeof(QueueElement));
    newElement->val = val;
    newElement->next = NULL;
    return newElement;
}

Queue *queueCreate(){
    Queue *newQueue = (Queue*)malloc(sizeof(Queue));
    newQueue->begin = NULL;
    newQueue->end = NULL;
    newQueue->size = 0;
    return newQueue;
}

void queuePush(Queue *queue, void *val){
    QueueElement *el = queueElementCreate(val);
    queue->size++;

    if(queue->end == NULL){
        queue->end = queue->begin = el;
    }
    else{
        queue->end->next = el;
        queue->end = queue->end->next;
    }
}

bool queueIsEmpty(Queue *queue){
    return queue->size==0;
}

void *queuePop(Queue *queue){
    queue->size--;

    void *res = queue->begin->val;
    if(queue->begin == queue->end){
        free(queue->begin);
        queue->begin = queue->end = NULL;
        return res;
    }
    QueueElement *newBegin = queue->begin->next;
    free(queue->begin);
    queue->begin = newBegin;
    return res;
}