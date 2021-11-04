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
#include <pthread.h>
#include <stdlib.h>

#include "include/node.h"

#define MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE



// Queue structs
static Node * _queue_head = NULL; // The oldest elements are closest to the head
static Node * _queue_tail = NULL; // The newest elements are closest to the tail
static int _queue_size = -1; // Size of the queue: -1 for invalid
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER; // Using static initializer: change if sharing between processes




/*
 * Params:
 *
 * Returns:
 * 
 * Description
 *      Initialize the queues, as well as the mutex to access the queues. Allocates head and tail placeholders, and constructs a queue of size 0. Call this only once per queue
*/
int queue_init(){
    pthread_mutex_lock(&qlock);

    // Queue head or tail placeholders have already been initialized
    if (is_queue_valid()){
        return 0;
    }

    _queue_head = malloc(sizeof(Node));
    if (!_queue_head){
        return 0;
    }

    _queue_head->id = -1;
    _queue_head->next = NULL;
    _queue_head->prev = NULL;

    _queue_tail = malloc(sizeof(Node));
    if (!_queue_tail){
        return 0;
    }

    _queue_tail->id = -2;
    _queue_tail->next = _queue_head;
    _queue_tail->prev = NULL;

    _queue_head->prev = _queue_tail;

    _queue_size = 0;

    pthread_mutex_unlock(&qlock);

    return 1;
}

int is_queue_valid(){
    pthread_mutex_lock(&qlock);
    if (!_queue_head || !_queue_tail || _queue_size == -1){
        return 0;
    }
    pthread_mutex_unlock(&qlock);
    return 1;
}

int queue_size(){
    pthread_mutex_lock(&qlock);
    int output = _queue_size;
    pthread_mutex_unlock(&qlock);

    return output;
}

int queue_put(Node* node){
    pthread_mutex_lock(&qlock);
    if (!is_queue_valid()){
        return 0;
    }

    Node * queue_tail_next = _queue_tail->next;
    _queue_tail->next = node;
    node->prev = _queue_tail;
    node->next = queue_tail_next;
    queue_tail_next->prev = node;

    _queue_size++;

    pthread_mutex_unlock(&qlock);
}

Node * queue_get(){
    pthread_mutex_lock(&qlock);

    Node * to_remove = _queue_head->prev;
    Node * to_remove_prev = _queue_head->prev->prev;
    to_remove_prev->next = _queue_head;
    _queue_head->prev = to_remove_prev;
    to_remove->prev = NULL;
    to_remove->next = NULL;

    _queue_size--;

    pthread_mutex_unlock(&qlock);

    return to_remove;
}

// Dont define these not atomic funcs yet
// Node * queue_get_head(){
//     return queue_head->prev;
// }

// Node * queue_get_tail(){
//     return queue_tail->next;
// }

void avada_Qdavra(){
    pthread_mutex_lock(&qlock);

    Node * temp = _queue_tail;
    
    while (temp){
        Node * temp_next = temp->next;
        free(temp);
        temp = temp_next;
    }

    _queue_size = -1;

    pthread_mutex_unlock(&qlock);
}

void node_print(Node * node){
    pthread_mutex_lock(&qlock);

    unprotected_node_print(node);

    pthread_mutex_unlock(&qlock);
}

void unprotected_node_print(Node * node){
    printf("{ID: %d}", node->id);
}

void queue_print(){
    pthread_mutex_lock(&qlock);

    Node * temp = _queue_tail;

    while (temp){
        unprotected_node_print(temp);
        if (temp != _queue_head){
            printf(" -> ");
        }
        temp = temp->next;
    }

    printf("\n");

    pthread_mutex_unlock(&qlock);
}

void main(){
    pthread_mutexattr_settype(&qlock, MUTEX_TYPE); 
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
