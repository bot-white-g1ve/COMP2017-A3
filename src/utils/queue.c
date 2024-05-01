#include <utils/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug/debug.h>

struct Queue* createQueue() {
    /**
     * Initialize the queue
    */
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    if (queue == NULL) {
        d_print("queue.createQueue", "Memory allocation failed for creating the queue\n");
        exit(EXIT_FAILURE);
    }
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

void enqueue(struct Queue* queue, void* data) {
    /**
     * Enqueue the data
    */
    struct QueueNode* newNode = (struct QueueNode*)malloc(sizeof(struct QueueNode));
    if (newNode == NULL) {
        d_print("queue.enqueue", "Memory allocation failed for creating the queue node\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

void* queue_get(struct Queue* queue) {
    /**
     * Get the first data but NOT remove it from the queue
    */
    if (queue->front == NULL) {
        d_print("queue.queue_get", "Queue is empty.\n");
        exit(EXIT_FAILURE);
    }
    return queue->front->data;
}

void* dequeue(struct Queue* queue) {
    /**
     * Get the first data and remove it from the queue
    */
    if (queue->front == NULL) {
        d_print("queue.dequeue", "Queue is empty.\n");
        exit(EXIT_FAILURE);
    }
    struct QueueNode* temp = queue->front;
    void* data = temp->data;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return data;
}

void free_queue(struct Queue* queue) {
    /**
     * Free the queue
     * Haven't free the data in the queue
    */
    while (queue->front != NULL) {
        struct QueueNode* temp = queue->front;
        queue->front = queue->front->next;
        // free(temp->data);
        free(temp);
    }
    free(queue);
}

int is_queue_empty(struct Queue* queue) {
    /**
     * Check if the queue is empty
     * Return 1 if the queue is empty, otherwise return 0
     */
    return queue->front == NULL;
}