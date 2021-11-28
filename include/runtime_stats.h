#ifndef RUNTIME_STATS_H
#define RUNTIME_STATS_H

#include <stdbool.h>
#include "node.h"

extern pthread_mutex_t mlock;
extern volatile float max_execution_time; 
extern volatile int max_coverage_count;
extern char max_node_input[INPUT_SIZE]; 


bool is_interesting(char mutated_input[], double execution_time, unsigned int exit_status, unsigned char in[],unsigned int domain);

#endif