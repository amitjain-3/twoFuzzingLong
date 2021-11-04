#include "include/runtime_stats.h"
#include <time.h>
#include <stdio.h> 

static volatile clock_t begin = 0;
static volatile clock_t end = 0;

void capture_begin_time(){     
    begin = clock();
} 

void capture_end_time(){ 
    end = clock();
}

float get_execution_time(){ 
    float time_spent = (float)((end - begin) / CLOCKS_PER_SEC);
    return time;
}


