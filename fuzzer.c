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

    Node * prev;
    queue_get(&prev);
    node_print(prev); printf("\n");
    queue_print();
    free(prev);

    queue_get(&prev);
    node_print(prev); printf("\n");
    queue_print();
    free(prev);

    queue_get(&prev);
    queue_print();

    avada_Qdavra();
}
