#include "include/runtime_stats.h"
#include <time.h>
#include <stdio.h> 
#include <pthread.h> 
#include "include/node.h"
#include "include/test_prog.h"
#include <math.h>



static volatile clock_t begin = 0;
static volatile clock_t end = 0;
volatile float max_execution_time = 0; 
volatile int max_coverage_count = 0;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
#define PERCENT_CUTOFF 0.7 


/*void capture_begin_time(){     
    begin = clock();
} 

void capture_end_time(){ 
    end = clock();
}*/

float get_execution_time(clock_t begin, clock_t end){
    float time_spent = (float)((end - begin) / CLOCKS_PER_SEC);
    return time_spent;
}

bool input_entry(Node* mutated_node,double execution_time, unsigned int exit_status, unsigned char coverage[], unsigned int domain){ 
    int entry = 0; 

    pthread_mutex_lock(&mlock);
    int coverage_count = get_coverage_count(coverage);
    //printf("max coverage count: %d ---- mutated input coverage %d  ---- Min coverage threshold for entry %f \n",max_coverage_count, coverage_count,floor(max_coverage_count * PERCENT_CUTOFF));
    //check if meaningful input, i.e if execution time > max_execution_time
    if (domain == RUNTIME_DOMAIN && execution_time > max_execution_time * PERCENT_CUTOFF){ 
        entry = 1; 
        if (execution_time > max_execution_time)
            max_execution_time = execution_time; 
    } else if (domain == COVERAGE_DOMAIN && coverage_count > floor(max_coverage_count * PERCENT_CUTOFF)) {
        entry = 1;
        if (coverage_count > max_coverage_count)
            max_coverage_count = coverage_count;  
    }
    pthread_mutex_unlock(&mlock);
        
    if (entry){
        mutated_node->runtime = execution_time; 
        mutated_node->exit_status = exit_status;
        mutated_node->coverage = coverage_count;
        queue_sorted_put(mutated_node,domain);
        //printf("Adding node to queue \n");
        return true;
    } else {
        //printf("Removing node from queue \n");
        return false;
    } 
}


