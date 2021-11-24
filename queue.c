#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/node.h"

#define ERROR 0
#define SUCCESS 1


// Definiton of main queue structs. Include node.h for declaration in other files.
Node * _queue_head = NULL;                          // The oldest elements are closest to the head
Node * _queue_tail = NULL;                          // The newest elements are closest to the tail
int _queue_size = -1;                               // Size of the queue: -1 for invalid
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;  // Using static initializer: change if sharing between processes


int queue_size(){
    pthread_mutex_lock(&qlock);
    int output = _queue_size;
    pthread_mutex_unlock(&qlock);

    return output;
}


int is_queue_valid__nolocks(){
    if (!_queue_head || !_queue_tail || _queue_size == -1){
        return ERROR;
    }

    return SUCCESS;
}


void node_init(Node * node, int id){
    node->id = id;
    node->thread_pid = -1;  
    node->core_affinity = -1; 
    memset(node->input, 0, INPUT_SIZE);
    node->exit_status = -1;  
    node->runtime = -1;  
    node->next = NULL;
    node->prev = NULL;
}


int queue_init(){
    pthread_mutex_lock(&qlock);

    if (is_queue_valid__nolocks()){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }

    _queue_head = malloc(sizeof(Node));
    if (!_queue_head){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }
    node_init(_queue_head, -1);

    _queue_tail = malloc(sizeof(Node));
    if (!_queue_tail){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }
    node_init(_queue_tail, -2);

    _queue_tail->next = _queue_head;
    _queue_head->prev = _queue_tail;
    _queue_size = 0;

    pthread_mutex_unlock(&qlock);
    return SUCCESS;
}


int queue_put(Node* node){
    pthread_mutex_lock(&qlock);

    if (!is_queue_valid__nolocks()){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }

    Node * queue_tail_next = _queue_tail->next;
    _queue_tail->next = node;
    node->prev = _queue_tail;
    node->next = queue_tail_next;
    queue_tail_next->prev = node;

    _queue_size++;

    pthread_mutex_unlock(&qlock);
    return SUCCESS;
}


int queue_sorted_put(Node* node, int domain){
    pthread_mutex_lock(&qlock);

    if (!is_queue_valid__nolocks()){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }

    Node * temp = _queue_tail;
    if (domain == RUNTIME_DOMAIN){ 
        while ((temp->next != _queue_head) && (temp->next->runtime >= node->runtime)){
            temp = temp->next;
        }
    } else {
        while ((temp->next != _queue_head) && (temp->next->coverage >= node->coverage)){
            temp = temp->next;
        }
    }
        
    Node * temp_next = temp->next;
    temp->next = node;
    node->prev = temp;
    node->next = temp_next;
    temp_next->prev = node;

    _queue_size++;
    pthread_mutex_unlock(&qlock);

    return SUCCESS;
}


int queue_get(Node ** return_node){
    pthread_mutex_lock(&qlock);

    if (!is_queue_valid__nolocks()){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }

    if (_queue_size == 0){
        printf("ERROR: Removing from empty queue\n");
        *return_node = NULL;
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }

    Node * to_remove = _queue_tail->next;
    Node * to_remove_next = to_remove->next;

    _queue_tail->next = to_remove_next;
    to_remove_next->prev = _queue_tail;
    to_remove->prev = NULL;
    to_remove->next = NULL;

    _queue_size--;
    *return_node = to_remove;

    pthread_mutex_unlock(&qlock);
    return SUCCESS; 
}


void node_print(Node * node){
    pthread_mutex_lock(&qlock);
    node_print__nolocks(node);
    pthread_mutex_unlock(&qlock);

    return;
}


void node_print__nolocks(Node * node){
    printf("{ID: %d RUN_TIME: %f COVERAGE: %d}", node->id, node->runtime, node->coverage);
    return;
}


int queue_print(){
    pthread_mutex_lock(&qlock);

    if (!is_queue_valid__nolocks()){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }

    Node * temp = _queue_tail;

    while (temp){
        node_print__nolocks(temp);
        if (temp != _queue_head){
            printf(" -> ");
        }
        temp = temp->next;
    }

    printf("\n");

    pthread_mutex_unlock(&qlock);
    return SUCCESS;
}


int avada_Qdavra(){
    pthread_mutex_lock(&qlock);

    if (!is_queue_valid__nolocks()){
        pthread_mutex_unlock(&qlock);
        return ERROR;
    }

    Node * temp = _queue_tail;
    
    while (temp){
        Node * temp_next = temp->next;
        free(temp);
        temp = temp_next;
    }

    _queue_size = -1;

    pthread_mutex_unlock(&qlock);
    return SUCCESS;
}
