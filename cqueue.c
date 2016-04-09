/*
Problem
Implement a circular queue of integers of user-specified size using a simple array.
Provide routines to initialize(), enqueue() and dequeue() the queue. Make it thread safe.
Please implement this with your own code and do not use any class libraries or library calls.
*/

#include <stdlib.h>
#include "cqueue.h"

struct _Queue {
    int capacity;
    int head;
    int tail;
    unsigned int element_size;
    void **buffer;
};

Queue*  queue_initialize(unsigned int element_size, unsigned int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (NULL == queue) {
        return NULL;
    }

    queue->element_size = element_size;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->buffer = (void*)malloc(element_size * (1 + capacity));
    if (NULL == queue->buffer) {
        free(queue);
        return NULL;
    }

    return queue;
}

int queue_destroy(Queue *queue) {
    free(queue->buffer);
    free(queue);
    return 0;
}

void* queue_enqueue(Queue *queue) {
    if (queue_size(queue) >= queue->capacity) {
        return NULL;
    }
    int idx = (queue->tail * queue->element_size);
    void* tmp = &queue->buffer[idx];
    queue->tail = (queue->tail + 1) % (queue->capacity + 1);

    return tmp;
}

int queue_dequeue(Queue *queue, void **out_element) {
    if (queue_size(queue) <= 0) {
        return -1;
    }

    int idx = (queue->head * queue->element_size);
    *out_element = &queue->buffer[idx];
    queue->head = (queue->head + 1) % (queue->capacity + 1);
    return 0;
}

int queue_size(Queue *queue) {
    return (queue->tail - queue->head + (1 + queue->capacity)) % (1 + queue->capacity);
}