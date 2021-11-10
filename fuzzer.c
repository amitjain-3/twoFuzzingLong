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
#include <unistd.h>
#include "include/node.h"
#import <mach/thread_act.h>
#define NUM_THREADS 8

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

#if defined __APPLE__ || defined __unix__
    /*need to figure out how to set affinity, cpu_set_t does not exist. 
      doesnt look like we can set affinity explictly on OSX, 
      but can give hints to kernel which threads do not need to share L2 cache so that they can be scheduled appropriate */
#elif __linux__
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);
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
        usleep(250000); 
    }

    for( i = 0; i < processorCount; i++ ) {
        pthread_join(threads[i], NULL);
    }

    for( i = 0; i < processorCount-1; i++ ) {
        
        rc = pthread_create(&threads[i], NULL, rem, NULL);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
        usleep(250000);
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
