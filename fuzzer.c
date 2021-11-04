// Write mutate func
// Store interesting inputs in a queue
// Find a way to run external programs
// Measure the rntime
// Get the appropriate seed input

// Funcs that we need
// 1. Queue operations: get, put, remove, size, 
// 2. Mutate operations: bit flip, delete/add element, 
// 3. Run test program: get runtime, get crash status, etc, maybe coverage later
// 4. Define code structure: data structure operations, branch behaviour 

#include <stdio.h>
#include <stdlib.h>
#include "include/node.h"

// Elements are added after the tail, and removed from before the head.
static Node * queue_head = NULL; 
static Node * queue_tail = NULL; 
void node_print(Node * node);
// Create a placeholder element for head and tail: call this only once
int queue_init(){
    if (queue_head != NULL || queue_tail != NULL){
        return 0;
    }
    queue_head = malloc(sizeof(Node));
    if (!queue_head){
        return 0;
    }

    queue_head->id = -1;
    queue_head->next = NULL;
    queue_head->prev = NULL;

    queue_tail = malloc(sizeof(Node));
    if (!queue_tail){
        return 0;
    }

    queue_tail->id = -2;
    queue_tail->next = queue_head;
    queue_tail->prev = NULL;

    queue_head->prev = queue_tail;

    return 1;
}

void queue_put(Node* node){
    Node * queue_tail_next = queue_tail->next;
    queue_tail->next = node;
    node->prev = queue_tail;
    node->next = queue_tail_next;
    queue_tail_next->prev = node;
}

Node * queue_get(){
    Node * to_remove = queue_head->prev;
    Node * to_remove_prev = queue_head->prev->prev;
    to_remove_prev->next = queue_head;
    queue_head->prev = to_remove_prev;
    to_remove->prev = NULL;
    to_remove->next = NULL;

    return to_remove;
}

Node * queue_get_head(){
    return queue_head->prev;
}

Node * queue_get_tail(){
    return queue_tail->next;
}

void avada_Qdavra(){
    Node * temp = queue_tail;
    
    while (temp){
        Node * temp_next = temp->next;
        free(temp);
        temp = temp_next;
    }
}

void node_print(Node * node){
    printf("{ID: %d}", node->id);
}

void queue_print(){
    Node * temp = queue_tail;

    while (temp){
        node_print(temp);
        if (temp != queue_head){
            printf(" -> ");
        }
        temp = temp->next;
    }

    printf("\n");
}

void main(){
    queue_init();

    Node * node = malloc(sizeof(Node));
    node->id = 1;
    queue_put(node);
    queue_print();

    Node * node2 = malloc(sizeof(Node));
    node2->id = 2;
    queue_put(node2);
    queue_print();

    Node * prev = queue_get();
    node_print(prev);
    queue_print();
    free(prev);

    avada_Qdavra();
}
