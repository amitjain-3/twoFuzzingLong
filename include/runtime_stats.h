#ifndef RUNTIME_STATS_H
#define RUNTIME_STATS_H


void capture_begin_time(); 
void capture_end_time();
float get_execution_time(); 
void input_entry(float execution_time, unsigned int exit_status, int ID);


#endif