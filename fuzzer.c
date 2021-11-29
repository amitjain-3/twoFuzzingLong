#define _GNU_SOURCE

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "include/node.h"
#include "include/runtime_stats.h"
#include "include/test_prog.h"

#define COVERAGE_UPPER_MAX 9
#define NUM_MUTATION_FUNCS 6
#define NUM_THREADS 8

int total_loops = 0; 
pthread_mutex_t llock; 


void int_handler (int signum){
    // Destroy queue stuff
    avada_Qdavra();
    pthread_mutex_destroy(&qlock);
    printf("\nQuitting...\n");
    exit(0);
}

void _byte_flip(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    in[byte_num] ^= (unsigned)0xff;
}

void _bit_flip(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    int bit_num = rand() % 8;
    in[byte_num] ^= (unsigned)(0x1 << bit_num);
}

void _byte_set(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    in[byte_num] |= (unsigned)0xff;
}

void _bit_set(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    int bit_num = rand() % 8;
    in[byte_num] |= (unsigned)(0x1 << bit_num);
}

void _byte_clear(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    in[byte_num] &= ~(unsigned)0xff;
}

void _bit_clear(unsigned char in[INPUT_SIZE])
{
    int byte_num = rand() % INPUT_SIZE;
    int bit_num = rand() % 8;
    in[byte_num] &= ~(unsigned)(0x1 << bit_num);
}

int mutate(unsigned char in[INPUT_SIZE])
{
    typedef void (*func_t)(unsigned char[INPUT_SIZE]);

    func_t funcs[NUM_MUTATION_FUNCS] = {_byte_flip, _bit_flip, _byte_set, _bit_set, _byte_clear, _bit_clear};

    int func_num = rand() % NUM_MUTATION_FUNCS;
    funcs[func_num](in);
}

void print_hex(const unsigned char s[INPUT_SIZE])
{
    // for (int i; i < INPUT_SIZE; i++)
    //     printf("%x-", (unsigned int)s[i]);
    //     fflush(stdout);
    // printf("\n");

    FILE *fp;

    fp = fopen("max_results.bin","wb");  // w for write, b for binary

    fwrite(s, INPUT_SIZE, 1, fp); // write 10 bytes from our buffer
    fclose(fp);
}

void *show_stats(){ 
    
    int temp_loops; 
    int temp_queue_size = 0;
    double elapsed_time;
    // clock_t start = clock();

    struct timespec begin, end; 
    clock_gettime(CLOCK_REALTIME, &begin);

    // printf("------------------------------------\n"); 
    
    while (1){
        // clock_t end = clock();
        // elapsed_time = (end - start)/(double)CLOCKS_PER_SEC;

        pthread_mutex_lock(&qlock);
        temp_queue_size = _queue_size;
        pthread_mutex_unlock(&qlock);

        pthread_mutex_lock(&llock); 
        temp_loops = total_loops; 
        pthread_mutex_unlock(&llock);

        pthread_mutex_lock(&mlock);
        // printf("\r| Max Coverage (# of Branches):    %d| Max Execution Time:   %0.2f| Total number of Loops:   %d| Size of queue:   %d| CPU TIME:    %0.6f", 
        //     max_coverage_count, max_execution_time, temp_loops, temp_queue_size, elapsed_time);
        // printf("\r%d, %0.2f, %d, %d, %0.6f", 
        //     max_coverage_count, max_execution_time, temp_loops, temp_queue_size, elapsed_time);
        fflush(stdout);
        pthread_mutex_unlock(&mlock);

        if (max_coverage_count == COVERAGE_UPPER_MAX){
            clock_gettime(CLOCK_REALTIME, &end);
            long seconds = end.tv_sec - begin.tv_sec;
            long nanoseconds = end.tv_nsec - begin.tv_nsec;
            elapsed_time = seconds + nanoseconds*1e-9;

            printf("%d, %0.2f, %d, %d, %0.6f", 
            max_coverage_count, max_execution_time, temp_loops, temp_queue_size, elapsed_time);
            // printf("\n Found max coverage \n");
            // printf("------------------------------------\n"); 

            print_hex(max_node_input);

            // printf("------------------------------------\n"); 

            exit(0);
        }

        usleep(1*1e6);
    } 
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
            memcpy(max_node_input, node->input, INPUT_SIZE);
        }

        if (node->runtime > max_execution_time){ 
            max_coverage_count = node->coverage;
            memcpy(max_node_input, node->input, INPUT_SIZE);
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

    int domain = *((int *) fuzz_domain);
    double runtime; 
    unsigned char cov[COVERAGE_BYTE_SIZE];
    char mutated_input[INPUT_SIZE] = {0}; 

    int i = 0;
    while (1) //i++ < 500000)
    {   
        while (!queue_size()){
            usleep(1e3);
        };

        pthread_mutex_lock(&llock);
        total_loops+=1;
        pthread_mutex_unlock(&llock);

        // Get one input from queue, should we be removing the element from queue? 
        // If we want to keep, we need a way to track mutations so that we dont want to apply same one again
        queue_get(&curr);

        // Mutate it
        memcpy(mutated_input, curr->input, INPUT_SIZE);
        mutate(mutated_input);

        //collect data on mutated input
        int exit_status = run_test_program(curr->input, &runtime, cov);
               
        //enter input if interesting 
        if (is_interesting(mutated_input, runtime, exit_status, cov, domain)){
            Node * mutated_node = malloc(sizeof(Node));
            node_init(mutated_node, rand()%10000);
            memcpy(mutated_node->input, mutated_input, INPUT_SIZE);
            mutated_node->runtime = runtime; 
            mutated_node->exit_status = exit_status;
            mutated_node->coverage = get_coverage_count(cov);

            // queue_put(mutated_node);
            // free(curr);
            queue_sorted_put(mutated_node, domain);
        }
        // else {
            // queue_put(curr);
            // queue_sorted_put(curr, domain);
        // }
        // queue_put(curr);
        curr->coverage = 0;
        if (queue_size() < 500)
            // queue_put(curr);
            queue_sorted_put(curr, domain);
    }

    return NULL;
}

int main(int argc, char *argv[])
{   
    signal(SIGINT, int_handler);

    /*** Parse inputs ***/
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

    /*** First do some system profiling FOR LINUX ***/
    // Get num cores
    int processorCount = 1;
    // processorCount = sysconf(_SC_NPROCESSORS_ONLN);
    // printf("Number of logical cores available: %d\n", processorCount);
    //current_fuzz_inputs = malloc(sizeof(int)*processorCount);

    pthread_t *threads = malloc(sizeof(pthread_t) * (processorCount)); // Array of threads for each core
    pthread_t print_thread;
    cpu_set_t cpus; // ?

    pthread_attr_t attr; // ?
    struct sched_param param;
    pthread_create(&print_thread,NULL,show_stats,NULL);

    // Set the seed for the random number generator
    srand((unsigned) time(NULL));
    
    /*** Now setup the queue ***/
    queue_init();
    interesting_inputs_to_queue("inputs/inputs1.txt", domain);

    /*** Main thread creation- one for each core ***/
    for (int i = 0; i < processorCount; i++)
    {
        // Assign cpu mask here in cpus and use set affinity passing cpu to create attribute, pass attr to pthread_create on each loop

        // Add cpu to set
        CPU_ZERO(&cpus);
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
    for (int i = 0; i < 1; i++) {
        pthread_join(threads[i], NULL);
    }


    free(threads);

    // Destroy queue stuff
    avada_Qdavra();
    pthread_mutex_destroy(&qlock);
    pthread_attr_destroy(&attr);

    return 0;
}
