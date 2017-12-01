#include "queue.h"
#include <stdlib.h>

//Finds the tail of the queue
node * GetTail(node * queueItem){
    node * tail;
    
    tail = queueItem;
    
    if(tail != NULL){
        while(tail->prev != NULL){
            tail = queueItem->prev;
        }
    }
    return tail;
}

//Gets the head of the queue
node * GetHead(node * queueItem){
    node * head;
    
    head = queueItem;
    if(head != NULL) {
        while(head->next != NULL){
            head = head->next;
        }
    }
    return head;
}

//Inserts Item to back of the queue
void Enqueue(queue * q, void * item){
    
    if( q->head == NULL){
        q->head = (node *)malloc(sizeof(node));
        q->tail = q->head;
        q->head->item = item;
        q->head->prev = NULL;
        q->head->next = NULL;
        q->count = 1;
    }else{
        node * curr;
        q->tail->prev = (node *)malloc(sizeof(node));
        curr = q->tail->prev;
        curr->prev = NULL;
        curr->next = q->tail;
        curr->item = item;
        q->tail = curr;
        q->count++;
    }
    
}

//Removes an Item from the head of the queue
void * Dequeue(queue * q){
    void * item;
    node * old;
    
    old = q->head;
    item = q->head->item;
    q->head = old->prev;
    q->count--;
    old->next = NULL;
    old->prev = NULL;
    //free(old->item);
    free(old);
    
    

    return item;
    
    
}
