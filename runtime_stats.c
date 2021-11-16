#include "include/runtime_stats.h"
#include <time.h>
#include <stdio.h> 
#include <pthread.h> 
#include "include/node.h"


static volatile clock_t begin = 0;
static volatile clock_t end = 0;
static volatile float max_execution_time = 0; 
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

void input_entry(float execution_time, unsigned int exit_status, int ID, char *mutated_input){ 
    int entry = 0; 
    pthread_mutex_lock(&mlock);
    //check if meaningful input, i.e if execution time > max_execution_time
    if (execution_time > max_execution_time * PERCENT_CUTOFF){ 
        max_execution_time = execution_time; 
        entry = 1; 
    } 
    pthread_mutex_unlock(&mlock);
        
    if (entry){
        Node* node = malloc(sizeof(Node*)); 
        node->id = ID; 
        node->runtime = execution_time; 
        node->exit_status = exit_status;
        for (int i = 0; i<INPUT_SIZE; i++){ 
            node->input[i]=*(mutated_input+i);
        }
        queue_put(node);
    } else { 
        printf("discard input - does not extend runtime");
    } 
}


