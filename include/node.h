#ifndef NODE_H
#define NODE_H

#define INPUT_SIZE 100
#define RUNTIME_DOMAIN 0
#define COVERAGE_DOMAIN 1

#include <pthread.h>
#include <stdlib.h>


// The queue node struct to store the mutated inputs
typedef struct _node {
    int id;                     // A unique ID number for entry
    unsigned int thread_pid;    // PID of the thread node is run on
    unsigned int core_affinity; // Core to run on
    char input[INPUT_SIZE];     // The input to the program
    unsigned int exit_status;   // The exit status of the program run with these inputs
    float runtime;              // The runtime when run with these inputs
    unsigned int coverage; 
    struct _node * next;        // Next element in the queue
    struct _node * prev;        // Prev element in the queueu
} Node;

extern Node * _queue_head;      // The oldest elements are closest to the head
extern Node * _queue_tail;      // The newest elements are closest to the tail
extern int _queue_size;         // Size of the queue: -1 for invalid
extern pthread_mutex_t qlock;   // Using static initializer: change if sharing between processes

int avada_Qdavra();
int is_queue_valid__nolocks();
int queue_get(Node ** return_node);
int queue_init();
int queue_print();
int queue_put(Node* node);
int queue_size();
int queue_sorted_put(Node* node, int domain);
void node_init(Node * node, int id);
void node_print(Node * node);
void node_print__nolocks(Node * node);

#endif