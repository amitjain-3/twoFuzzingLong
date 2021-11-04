#ifndef NODE_H
#define NODE_H

#define INPUT_SIZE 100

extern pthread_mutex_t qlock;

// The queue struct to store the mutated inputs
typedef struct _node {
    int id; // A unique ID number for entry
    unsigned int thread_pid;
    unsigned int core_affinity; // Core to be run on

    char input[INPUT_SIZE]; // The input to the program

    // After running the program
    unsigned int exit_status;
    float runtime;

    struct _node * next; // Next element in the queue
    struct _node * prev; // Prev element in the queueu
} Node;

int queue_init();
int queue_put(Node* node);
Node * queue_get();
// Node * queue_get_head();
// Node * queue_get_tail();
void avada_Qdavra();
void node_print(Node * node);
void queue_print();

#endif