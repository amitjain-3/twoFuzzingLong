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
#define NUM_THREADS 8
#define NUM_MUTATION_FUNCS 2


void _byte_flip(unsigned char in[INPUT_SIZE]){
    int byte_num = rand() % INPUT_SIZE;
    in[byte_num] ^= 0xff;
}


void _bit_flip(unsigned char in[INPUT_SIZE]){
    int byte_num = rand() % INPUT_SIZE;
    int bit_num = rand() % 8;
    in[byte_num] ^= (0x1<<bit_num);
}

typedef void (*func_t)(unsigned char [INPUT_SIZE]);

int mutate (unsigned char in[INPUT_SIZE]){
    func_t funcs[NUM_MUTATION_FUNCS] = {_byte_flip, _bit_flip};

    int func_num = rand() % NUM_MUTATION_FUNCS;
    funcs[func_num](in);
}

void print_hex(const unsigned char s[INPUT_SIZE])
{
  while(*s)
    printf("%02x", (unsigned int) *s++);
  printf("\n");
}

void main(){
    time_t t; srand((unsigned) time(&t));

    unsigned char test[100] = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n";
    mutate(test);
    print_hex(test);
}

void *add(void* num){
    Node * node = malloc(sizeof(Node));
    node->id = (int)num;
    queue_put(node);

    pthread_exit(NULL);
}


void *rem(){
    Node * prev;
    queue_get(&prev);
    node_print(prev); printf("\n");
    free(prev);

    pthread_exit(NULL);
}

void _main(){
    queue_init();

   
    // queue_get(&prev);
    // node_print(prev); printf("\n");
    // queue_print();
    // free(prev);

    // queue_get(&prev);
    // queue_print();
    pthread_t threads[NUM_THREADS];

    int rc;
    int i;
    for( i = 0; i < NUM_THREADS; i++ ) {
        
        rc = pthread_create(&threads[i], NULL, add, (void *)i);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    for( i = 0; i < NUM_THREADS; i++ ) {
        pthread_join(threads[i], NULL);
    }

    for( i = 0; i < NUM_THREADS-1; i++ ) {
        
        rc = pthread_create(&threads[i], NULL, rem, NULL);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    for( i = 0; i < NUM_THREADS-1; i++ ) {
        pthread_join(threads[i], NULL);
    }

    queue_print();
    avada_Qdavra();
    pthread_mutex_destroy(&qlock);
    pthread_exit(NULL);


    return;
}
