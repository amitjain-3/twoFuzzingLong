#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#ifdef __APPLE__
    #include <mach/thread_act.h>
#endif

#include "include/node.h"
#include "include/runtime_stats.h"

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


int determineCoreCount(){ 

}


void interesting_inputs_to_queue(char * filename){
    FILE * fp = fopen(filename, "r");
    char buf[INPUT_SIZE] = {0};

    while (fscanf(fp, "%s", buf) != EOF){
        Node * node = malloc(sizeof(Node));
        node_init(node, rand() % 10000);

        // Copy 99 chars + add \0 to the end
        memcpy(node->input, buf, INPUT_SIZE-1);
        node->input[99] = '\0';

        // Run on the input and get runtime
        int runtime = rand() % 10000;
        node->runtime = runtime;

        queue_sorted_put(node);
        // printf("%s\n", node->input);
    }

    fclose(fp);
    return;
}


void * fuzz_loop(){
    Node * curr;
    int i = 0;

    // Random calcs--- temporary
    volatile int j = 0;
    volatile int k = 1212;
    while (i++ < 100){
        j = (k + 1230) / k; // random calc

        // Get one input from queue
        queue_get(&curr);

        // Mutate it
        mutate(curr->input);
        curr->runtime = rand() % rand();

        // Check if interesting and add
        queue_sorted_put(curr);

        // free(curr); 
        usleep(0.1 * 1e6);
    }

    return NULL;
}


int main(){
    /*** First do some system profiling FOR LINUX ***/
    // Number of cores
    int processorCount = 1; // Default 
    processorCount = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of logical cores available: %d\n", processorCount);

    pthread_t * threads = malloc(sizeof(pthread_t) * processorCount); // Array of threads for each core

    cpu_set_t cpus; // ?
    CPU_ZERO(&cpus); // TODO Changed pos to here outside the loop. Right?

    pthread_attr_t attr; // ?
    struct sched_param param;

    /*** Now setup the queue ***/
    queue_init();
    interesting_inputs_to_queue("int_inputs.txt");

    /*** Main thread creation- one for each core ***/
    for(int i = 0; i < 1; i++){
        // Assign cpu mask here in cpus and use set affinity passing cpu to create attribute, pass attr to pthread_create on each loop

        // Add cpu to set
        CPU_SET(i, &cpus);

        // Set the affinity of thread to core
        pthread_attr_init (&attr);

        // Set thread priority
        pthread_attr_getschedparam (&attr, &param);
        param.sched_priority += 0; // Custom prio level?
        pthread_attr_setschedparam (&attr, &param);

        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        int rc = pthread_create(&threads[i], &attr, fuzz_loop, NULL);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    /*** Cleanup ***/
    // Join each thread
    for(int i = 0; i < 1; i++) {
        pthread_join(threads[i], NULL);
    }

    queue_print();


    free(threads);

    // Destroy queue stuff
    avada_Qdavra();
    pthread_mutex_destroy(&qlock);
    pthread_attr_destroy(&attr);
    
    return 0;
}


// void some_old_main_stuff(){
//     int processorCount = 1; //default 
//     pthread_attr_t *affinity_attr = NULL; 

//     processorCount = sysconf(_SC_NPROCESSORS_ONLN);
//     printf("Number of logical cores: %d\n", processorCount);


// #ifdef __APPLE__
//     /*need to figure out how to set affinity, cpu_set_t does not exist. 
//       doesnt look like we can set affinity explictly on OSX, 
//       but can give hints to kernel which threads do not need to share L2 cache so that they can be scheduled appropriate */
//     printf("Still working on this \n");
// #elif __linux__
//     pthread_attr_t attr;
//     cpu_set_t cpus;
//     pthread_attr_init(&attr);
//     printf("initialized attribute \n");
// #endif
    
//     pthread_t threads[processorCount];

//     int rc;
//     int i;
//     for( i = 0; i < processorCount; i++ ) {
//         //assign cpu mask here in cpus and use set affinity passing cpu to create attribute, pass attr to pthread_create on each loop (works for linux - need different way for OSX/unix)
// #ifdef __linux__
//         CPU_ZERO(&cpus);
//         CPU_SET(i, &cpus);
//         pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
//         affinity_attr = &attr;
// #endif
//         rc = pthread_create(&threads[i], affinity_attr, add, (void *)i);
//         if (rc) {
//             printf("Error:unable to create thread, %d\n", rc);
//             exit(-1);
//         }
//         //usleep(250000); 
//     }

//     for( i = 0; i < processorCount; i++ ) {
//         pthread_join(threads[i], NULL);
//     }

//     for( i = 0; i < processorCount-3; i++ ) {
// #ifdef __linux__
//         CPU_ZERO(&cpus);
//         CPU_SET(i, &cpus);
//         pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
//         affinity_attr = &attr;
// #endif
//         rc = pthread_create(&threads[i], affinity_attr, rem, NULL);
//         if (rc) {
//             printf("Error:unable to create thread, %d\n", rc);
//             exit(-1);
//         }
//         //usleep(250000);
//     }

//     for( i = 0; i < processorCount-3; i++ ) {
//         pthread_join(threads[i], NULL);
//     }

//     queue_print();
//     avada_Qdavra();
    
//     // Execution time Test
//     /*double time_spent = 0.0;
//     clock_t begin = clock();
//     // do some stuff here
//     unsigned char input[100] = "000000000000321000000000000000000000000000000000000000000000006540000000000000000000000000000000000\n";
//     //selection_sort(input);
//     sleep(3);
//     clock_t end = clock();
//     // calculate elapsed time by finding difference (end - begin) and
//     // dividing the difference by CLOCKS_PER_SEC to convert to seconds
//     time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
//     printf("The elapsed time is %f seconds \n", time_spent);
//     printf("The clocks elapsed is %lu \n", (end-begin));*/

//     pthread_mutex_destroy(&qlock);
//     pthread_exit(NULL);

//     return 0;
// }

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