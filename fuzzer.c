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
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/node.h"
#ifdef __APPLE__
#include <mach/thread_act.h>
#endif
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

int determineCoreCount(){ 

}

int main(){
    queue_init();

    int processorCount = 1; //default 
    pthread_attr_t *affinity_attr = NULL; 
    // queue_get(&prev);
    // node_print(prev); printf("\n");
    // queue_print();
    // free(prev);

    // queue_get(&prev);
    // queue_print();
    processorCount = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of logical cores: %d\n", processorCount);

#ifdef __APPLE__
    /*need to figure out how to set affinity, cpu_set_t does not exist. 
      doesnt look like we can set affinity explictly on OSX, 
      but can give hints to kernel which threads do not need to share L2 cache so that they can be scheduled appropriate */
    printf("Still working on this");
#elif __linux__
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);
    printf("initialized attribute \n");
#endif
    
    pthread_t threads[processorCount];

    int rc;
    int i;
    for( i = 0; i < processorCount; i++ ) {
        //assign cpu mask here in cpus and use set affinity passing cpu to create attribute, pass attr to pthread_create on each loop (works for linux - need different way for OSX/unix)
#ifdef __linux__
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        affinity_attr = &attr;
#endif
        rc = pthread_create(&threads[i], affinity_attr, add, (void *)i);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
        //usleep(250000); 
    }

    for( i = 0; i < processorCount; i++ ) {
        pthread_join(threads[i], NULL);
    }

    for( i = 0; i < processorCount-1; i++ ) {
#ifdef __linux__
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        affinity_attr = &attr;
#endif
        rc = pthread_create(&threads[i], affinity_attr, rem, NULL);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
        //usleep(250000);
    }

    for( i = 0; i < processorCount-1; i++ ) {
        pthread_join(threads[i], NULL);
    }

    queue_print();
    avada_Qdavra();
    pthread_mutex_destroy(&qlock);
    pthread_exit(NULL);

    return 0;
}
