#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>


#include "include/node.h"
#include "include/runtime_stats.h"
#include "include/test_prog.h"

#define NUM_THREADS 8
#define NUM_MUTATION_FUNCS 2
#define COVERAGE_UPPER_MAX 9

int total_loops = 0; 
pthread_mutex_t llock; 

//char *current_fuzz_inputs;

void _byte_flip(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    in[byte_num] ^= 0xff;
}

void _bit_flip(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    int bit_num = rand() % 8;
    in[byte_num] ^= (0x1 << bit_num);
}

typedef void (*func_t)(unsigned char[INPUT_SIZE]);

int mutate(unsigned char in[INPUT_SIZE])
{
    func_t funcs[NUM_MUTATION_FUNCS] = {_byte_flip, _bit_flip};

    int func_num = rand() % NUM_MUTATION_FUNCS;
    funcs[func_num](in);
}

void print_hex(const unsigned char s[INPUT_SIZE])
{
    while (*s)
        printf("%02x", (unsigned int)*s++);
    printf("\n");
}

int determineCoreCount()
{
}

void *show_stats(){ 
    printf("------------------------------------\n"); 
    int temp_loops; 
    int temp_queue_size = 0;
    double elapsed_time;
    clock_t start = clock();
    
    while (1){
        clock_t end = clock();
        elapsed_time = (end - start)/(double)CLOCKS_PER_SEC;
        pthread_mutex_lock(&qlock);
        temp_queue_size = _queue_size;
        pthread_mutex_unlock(&qlock);
        pthread_mutex_lock(&llock); 
        temp_loops = total_loops; 
        pthread_mutex_unlock(&llock);
        pthread_mutex_lock(&mlock);
        //fflush(stdout);
        printf("\r| Max Coverage (# of Branches):    %d| Max Execution Time:   %0.2f| Total number of Loops:   %d| Size of queue:   %d| CPU TIME:    %0.6f",max_coverage_count, max_execution_time,temp_loops,temp_queue_size,elapsed_time);
        fflush(stdout);
        //fflush(stdout);
        //printf("\r| Max Coverage count:             %d  |",max_coverage_count);
        //fflush(stdout);
        //printf("\r------------------------------------");
        pthread_mutex_unlock(&mlock);
        if (max_coverage_count == COVERAGE_UPPER_MAX){
            printf("\n Found max coverage \n");
            exit(0);
        }
        usleep(1*1e6);
    } 
    //printf("| Number of threads:               |");
}

void interesting_inputs_to_queue(char *filename, int domain)
{
    FILE *fp = fopen(filename, "r");
    char buf[INPUT_SIZE] = {0};
    unsigned char cov[COVERAGE_BYTE_SIZE];
    double runtime; 

    while (fscanf(fp, "%s", buf) != EOF)
    {
        Node *node = malloc(sizeof(Node));
        node_init(node, rand() % 10000);

        // Copy 99 chars + add \0 to the end
        memcpy(node->input, buf, INPUT_SIZE - 1);
        node->input[99] = '\0';

        // Run on the input and get runtime
        run_test_program(node->input, &runtime , cov);
        node->runtime = runtime;
        node->coverage = get_coverage_count(cov);
        if (node->coverage > max_coverage_count){ 
            max_coverage_count = node->coverage;
        }
        //printf("Coverage test input %d, max coverage %d\n",node->coverage, max_coverage_count);
        queue_sorted_put(node, domain);
        // printf("%s\n", node->input);coverage
    }

    fclose(fp);
    return;
}

void *fuzz_loop(void *fuzz_domain)
{
    Node *curr;
    int i = 0;
    int domain = *((int *) fuzz_domain);
    double runtime; 
    unsigned char cov[COVERAGE_BYTE_SIZE];
    // Random calcs--- temporary
    volatile int j = 0;
    volatile int k = 1212;
    while (i++ < 1000)
    {
        pthread_mutex_lock(&llock);
        total_loops+=1;
        pthread_mutex_unlock(&llock);

        j = (k + 1230) / k; // random calc

        // Get one input from queue, should we be removing the element from queue? 
        //if we want to keep, we need a way to track mutations so that we dont want to apply same one again
        queue_get(&curr);

        // Mutate it
        mutate(curr->input);

        //collect data on mutated input
        int exit_status = run_test_program(curr->input,&runtime,cov);
        
        //enter input if interesting 
        input_entry(curr,runtime,exit_status,cov,domain);
        
        //curr->runtime = rand() % rand();
        // Check if interesting and add
        //queue_sorted_put(curr, domain);

        // free(curr);
        usleep(0.1 * 1e6);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    /*** First do some system profiling FOR LINUX ***/
    // Number of cores
    int domain;

    if (argc != 2)
    {
        printf("Incorrect number of arugments. Please pass one domain as argument (runtime or coverage). \n");
        return 0;
    }
    else if (strcmp(argv[1], "runtime") != 0 && strcmp(argv[1], "coverage") != 0)
    {
        printf("Argument should be either \"runtime\" or \"coverage\". \n");
        return 0;
    }
    else if (!strcmp(argv[1], "runtime"))
    {
        domain = RUNTIME_DOMAIN;
    }
    else
    {
        domain = COVERAGE_DOMAIN;
    }
    int processorCount = 1;
    processorCount = sysconf(_SC_NPROCESSORS_ONLN);
    processorCount = 1;
    printf("Number of logical cores available: %d\n", processorCount);
    //current_fuzz_inputs = malloc(sizeof(int)*processorCount);

    pthread_t *threads = malloc(sizeof(pthread_t) * (processorCount)); // Array of threads for each core
    pthread_t print_thread;
    cpu_set_t cpus; // ?

    pthread_attr_t attr; // ?
    struct sched_param param;
    pthread_create(&print_thread,NULL,show_stats,NULL);
    /*** Now setup the queue ***/
    queue_init();
    interesting_inputs_to_queue("inputs/inputs1.txt", domain);

    /*** Main thread creation- one for each core ***/
    for (int i = 0; i < processorCount; i++)
    {
        // Assign cpu mask here in cpus and use set affinity passing cpu to create attribute, pass attr to pthread_create on each loop

        // Add cpu to set
        CPU_ZERO(&cpus); // TODO Changed pos to here outside the loop. Right?
        CPU_SET(i, &cpus);

        // Set the affinity of thread to core
        pthread_attr_init(&attr);

        // Set thread priority
        pthread_attr_getschedparam(&attr, &param);
        param.sched_priority += 0; // Custom prio level?
        pthread_attr_setschedparam(&attr, &param);
        
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        int rc = pthread_create(&threads[i], &attr, fuzz_loop, &domain);
        if (rc)
        {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    /*** Cleanup ***/
    // Join each thread
    for (int i = 0; i < 1; i++)
    {
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