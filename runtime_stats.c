#include <math.h>
#include <pthread.h> 
#include <stdio.h> 
#include <string.h>
#include <time.h>

#include "include/node.h"
#include "include/runtime_stats.h"
#include "include/test_prog.h"

#define PERCENT_CUTOFF 0.1

volatile float max_execution_time = 0; 
volatile int max_coverage_count = 0;
char max_node_input[INPUT_SIZE] = {0}; 

pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;


bool is_interesting(char mutated_input[], double execution_time, unsigned int exit_status, unsigned char coverage[], unsigned int domain){ 
    bool entry = false; 

    pthread_mutex_lock(&mlock);
    int coverage_count = get_coverage_count(coverage);
    //printf("max coverage count: %d ---- mutated input coverage %d  ---- Min coverage threshold for entry %f \n",max_coverage_count, coverage_count,floor(max_coverage_count * PERCENT_CUTOFF));
    
    //check if meaningful input, i.e if execution time > max_execution_time
    if (domain == RUNTIME_DOMAIN && execution_time >= max_execution_time * PERCENT_CUTOFF){ 
        entry = true; 
        if (execution_time > max_execution_time){
            max_execution_time = execution_time; 
            memcpy(max_node_input, mutated_input, INPUT_SIZE);
        }

    } else if (domain == COVERAGE_DOMAIN && coverage_count >= floor(max_coverage_count * PERCENT_CUTOFF)) {
        entry = true;
        if (coverage_count > max_coverage_count){
            max_coverage_count = coverage_count; 
            memcpy(max_node_input, mutated_input, INPUT_SIZE);
        } 
    }

    pthread_mutex_unlock(&mlock);
    return entry;
}


