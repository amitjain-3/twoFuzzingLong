#include "include/runtime_stats.h"
#include <time.h>
#include <stdio.h> 
#include <pthread.h> 
#include "include/node.h"


static volatile clock_t begin = 0;
static volatile clock_t end = 0;

static volatile float max_execution_time = 0; 


void capture_begin_time(){     
    begin = clock();
} 

void capture_end_time(){ 
    end = clock();
}

float get_execution_time(){ 
    float time_spent = (float)((end - begin) / CLOCKS_PER_SEC);
    return time_spent;
}

void input_entry(float execution_time, unsigned int exit_status, int ID, char *mutated_input){ 
    pthread_mutex_lock(&qlock);
    //check if meaningful input, i.e if execution time > max_execution_time
    if (execution_time > max_execution_time){ 
        max_execution_time = execution_time; 
        Node* node = malloc(sizeof(Node*)); 
        node->id = ID; 
        node->runtime = execution_time; 
        node->exit_status = exit_status;
        for (int i = 0; i<INPUT_SIZE; i++){ 
            node->input[i]=*(mutated_input+i)
        }
        queue_put(node);
    } 
    pthread_mutex_unlock(&qlock);
}


