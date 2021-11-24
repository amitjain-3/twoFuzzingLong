#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/node.h"
#include "include/runtime_stats.h"
#include "include/test_prog.h"

#define NUM_THREADS 8
#define NUM_MUTATION_FUNCS 2


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
        printf("Coverage test input %d \n",node->coverage);
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
    // Random calcs--- temporary
    volatile int j = 0;
    volatile int k = 1212;
    while (i++ < 100)
    {
        j = (k + 1230) / k; // random calc

        // Get one input from queue
        queue_get(&curr);

        // Mutate it
        mutate(curr->input);
        curr->runtime = rand() % rand();

        // Check if interesting and add
        queue_sorted_put(curr, domain);

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

    int processorCount = 1; // Default
    processorCount = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of logical cores available: %d\n", processorCount);

    pthread_t *threads = malloc(sizeof(pthread_t) * processorCount); // Array of threads for each core

    cpu_set_t cpus; // ?

    pthread_attr_t attr; // ?
    struct sched_param param;

    /*** Now setup the queue ***/
    queue_init();
    interesting_inputs_to_queue("inputs/inputs1.txt", domain);

    /*** Main thread creation- one for each core ***/
    for (int i = 0; i < 1; i++)
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