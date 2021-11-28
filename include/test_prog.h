#ifndef TEST_PROG_H
#define TEST_PROG_H

#define COVERAGE_BYTE_SIZE 5

int get_coverage_count(unsigned char coverage_out[]);
int run_test_program(unsigned char in[], double * runtime, unsigned char coverage_out[]);

#endif