#ifndef CQUEUE_H
#define CQUEUE_H

typedef struct _Queue Queue;

Queue* queue_initialize(unsigned int element_size, unsigned int capacity);
int queue_destroy(Queue* queue);
void* queue_enqueue(Queue* queue);
int queue_dequeue(Queue* queue, void **out_element);
int queue_size(Queue* queue);

#endif