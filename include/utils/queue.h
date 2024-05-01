#ifndef QUEUE_H
#define QUEUE_H

struct QueueNode {
    void* data;
    struct QueueNode* next;
};

struct Queue {
    struct QueueNode* front;
    struct QueueNode* rear;
};

extern struct Queue* createQueue();
extern void enqueue(struct Queue* queue, void* data);
extern void* queue_get(struct Queue* queue);
extern void* dequeue(struct Queue* queue);
extern void free_queue(struct Queue* queue);
extern int is_queue_empty(struct Queue* queue);

#endif