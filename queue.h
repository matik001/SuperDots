#ifndef QUEUE_H
#define QUEUE_H
#include <stdlib.h>
#include <stdbool.h>

typedef struct QueueElement QueueElement;
struct QueueElement{
    void *val;
    QueueElement *next;
};

typedef struct Queue Queue;
struct Queue{
    int size;
    QueueElement *begin, *end;
};
Queue *queueCreate();
void queuePush(Queue *queue, void *val);
bool queueIsEmpty(Queue *queue);
void *queuePop(Queue *queue);





#endif