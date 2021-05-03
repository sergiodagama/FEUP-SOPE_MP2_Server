#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct message { int rid; pid_t pid; pthread_t tid; int tskload; int tskres; } message_t;

/**
 * @brief Queue struct
 */
typedef struct Queue{
    int front, back, size;
    unsigned int maxCapacity;
    message_t* msg;

} queue;

queue* createQueue( unsigned  maxCapacity );
bool isEmpty( queue* queue );
bool isFull( queue* queue );
void insert( queue* queue, message_t content );
message_t pop( queue* queue );
message_t front( queue* queue);
message_t back( queue* queue);



#endif
