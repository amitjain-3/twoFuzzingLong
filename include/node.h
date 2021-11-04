#ifndef NODE_H
#define NODE_H

#define INPUT_SIZE 100

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

int add_fifo(Node * node); //add input to queue if it is interesting

Node* get_fifo(void); //return first input from queue if

#endif