#ifndef QUEUE_H
#define QUEUE_H

//Initializes Queue

typedef struct node{
    char item;
    struct node * prev;
    struct node * next;
    
}node;

typedef struct queue{
    node * head;
    node * tail;
    int count;
}queue;

//Inserts Item to back of the queue
void Enqueue(queue * q, char item);

//Removed an item from the head of the queue
char Dequeue(queue * q);

//Finds the tail of the queue
node * GetTail(node * queueItem);

//Finds the head of the queue
node * GetHead(node * queueItem);
#endif

