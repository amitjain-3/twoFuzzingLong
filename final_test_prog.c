#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "include/node.h"
#include "include/test_prog.h"

// Bitmap storing coverage for each path->bit-index
unsigned char coverage[COVERAGE_BYTE_SIZE] = {0};

#define SET_COV(pos) (coverage[pos/8] |= 1<<(pos%8));

void single_loop(unsigned char in[], int udelay){
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        volatile int x = 0;
        x += 1;
        usleep(udelay);
    }
}

void double_loop(unsigned char in[], int udelay){
    for (int i = 0; i < INPUT_SIZE; i++){   
        if (in[2] <= 0x7f){
            SET_COV(0);
            continue;
        }

        for (int j = i; j < INPUT_SIZE; j++)
        {
            volatile int x = 0;
            x += 1;
            usleep(udelay);
        }
    }
}

void triple_loop(unsigned char in[], int udelay){
    for (int i = 0; i < INPUT_SIZE; i++){
        if (in[3] <= 0x7f){
            SET_COV(1);
            continue;
        }

        for (int j = i; j < INPUT_SIZE; j++){   
            if (in[4] <= 0x7f){
                SET_COV(2);
                continue;
            }
            for (int k = j; k < INPUT_SIZE; k++){
                volatile int x = 0;
                x += 1;
            }
            usleep(udelay);
        }
    }
}

void no_loop(unsigned char in[], int udelay){ 
    int unsigned max = in[0], min = in[0];
    for(int i = 0; i < INPUT_SIZE-1; i++){ 
        if ((in[i]) > max){ 
            max = in[i]; 
        }
        if ((in[i]) < min){ 
            min = in[i]; 
        }
        usleep(udelay);
    }
    //printf(" Max %d and Min %d \n",max,min);

    if (max >= 0x70){ //need a bit shift into one of 2 MSB to hit this
        SET_COV(7);
    }

    if (min != 0x30){ //atleast all 0's modified
        SET_COV(8);
    }

    if (in[1] > min){ 
        SET_COV(9);
    }
     
}



int _main(unsigned char in[]){

    int udelay = in[1]; // in milliseconds now
    if (in[0] <= 0xff){
        SET_COV(3);
        single_loop(in, udelay);
    }
    if (in[0] <= 0x7f){
        SET_COV(4);
        double_loop(in, udelay);
    }
    if (in[0] <= 0x30){
        SET_COV(5);
        triple_loop(in, udelay);
    }
    if(in[0] <= 0x34){ 
        SET_COV(6); 
        no_loop(in, udelay);
    }

}

int run_test_program(unsigned char in[], double * runtime, unsigned char coverage_out[]){
    //Start timer
    struct timespec begin, end; 
    clock_gettime(CLOCK_REALTIME, &begin);

    // Run program
    _main(in);

    // End timer
    clock_gettime(CLOCK_REALTIME, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;

    // Return the runtime
    *runtime = seconds + nanoseconds*1e-9;
    memcpy(coverage_out, coverage, COVERAGE_BYTE_SIZE);

    // Return the exit code?
    return 1;
}   

int get_coverage_count(unsigned char coverage_out[]){
    int count = 0;
    for (int i = 0; i < COVERAGE_BYTE_SIZE; i++){
        while (coverage[i] != 0){
            if (coverage[i] & 0x01){
                count++;
            }
            
            coverage[i] >>= 1;
        }
    }

    return count;
}

/*int main(){
    unsigned char in[6] = "\x00\xff\xff\xff\xff\0";
    double runtime = 0;
    unsigned char cov[COVERAGE_BYTE_SIZE];
    run_test_program(in, &runtime, cov);

    // Print coverage
    for (int i = 0; i < COVERAGE_BYTE_SIZE; i++){
        printf("%x ", cov[i]);
    }
    printf("\nRuntime: %f\n", runtime);
    printf("\nCov: %d\n", get_coverage_count(cov));
    
    return 0;
}*/