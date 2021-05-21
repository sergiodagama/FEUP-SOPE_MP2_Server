#include "queue.h"

/**
 * @brief Initialize the queue, given the desired maximum capacity. Allocating memory for the array of structs,
 * that will store the information to be handled or already handled by the server.
 * @param maxCapacity - The maximum capacity desired for the queue
 * @return queue that contains message_t items
 */
queue* createQueue( unsigned  maxCapacity ){
    queue* newQueue = (queue*) malloc( sizeof( queue ) );
    newQueue->maxCapacity = maxCapacity;
    newQueue->front = newQueue->size = 0;

    newQueue->back = maxCapacity - 1;
    newQueue->msg = (message_t **) malloc(newQueue->maxCapacity * sizeof(message_t));

    return newQueue;
}

/**
 * @brief Check if the queue is empty
 * @param queue
 * @return true if empty, false otherwise
 */
bool isEmpty( queue* queue ){
    return (queue->size == 0);
}


/**
 * @brief Check if the queue is full
 * @param queue
 * @return true if full, false otherwise
 */
bool isFull( queue* queue ){
    return (queue->size == queue->maxCapacity);
}

/**
 * @brief Insert a message_t item inside the queue
 * @param queue
 * @param content
 */
void insert( queue* queue, message_t* content ){
    if( isFull(queue) ) {
        fprintf(stderr, "Queue full\n");
        return;
    }

    queue->back = (queue->back + 1) % queue->maxCapacity;
    queue->msg[queue->back] = content;
    queue->size++;
}

/**
 * @brief Remove the front item of the queue (FIFO), and consequently update the queue current size (size)
 * and the front item
 * @param queue
 * @return message_t item equal to the front item, if the queue is empty return a message_t item with pid == -1
 */
message_t* pop( queue* queue ){
    if(isEmpty(queue) ) return NULL;

    message_t *poped = queue->msg[queue->front];

    queue->front = (queue->front + 1) % queue->maxCapacity;
    queue->size--;
    return poped;
}

/**
 * @brief Queue front item
 * @param queue
 * @return message_t item equal to the front item, if the queue is empty return a message_t item with pid == -1
 */
message_t* front( queue* queue){
    if(isEmpty( queue ) ) return NULL;
    return queue->msg[queue->front];
}

/**
 * @brief Queue front item
 * @param queue
 * @return message_t item equal to the front item, if the queue is empty return a message_t item with pid == -1
 */
message_t* back( queue* queue){
    if(isEmpty( queue ) ) return NULL;
    return queue->msg[queue->back];
}


