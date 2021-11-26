#ifndef RUNTIME_STATS_H
#define RUNTIME_STATS_H

void capture_begin_time(); 
void capture_end_time();
float get_execution_time(); 
bool input_entryinput_entry(Node* mutated_node,double execution_time, unsigned int exit_status, unsigned char in[],unsigned int domain);

#endif