#define _GNU_SOURCE
#include <stdio.h> 
#include "include/runtime_stats.h"
#include <asm/unistd.h>
#include "include/node.h"
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <linux/perf_event.h>    /* Definition of PERF_* constants */
#include <linux/hw_breakpoint.h> /* Definition of HW_* constants */
#include <sys/syscall.h>         /* Definition of SYS_* constants */
#include <unistd.h>
#include <time.h>

void quicksort(unsigned char number[],int first,int last){
    //https://beginnersbook.com/2015/02/quicksort-program-in-c/
   int i, j, pivot;
   unsigned char temp;

   if(first<last){
      pivot=first;
      i=first;
      j=last;

      while(i<j){
         while(number[i]<=number[pivot]&&i<last)
            i++;
         while(number[j]>number[pivot])
            j--;
         if(i<j){
            temp=number[i];
            number[i]=number[j];
            number[j]=temp;
         }
      }

      temp=number[pivot];
      number[pivot]=number[j];
      number[j]=temp;
      quicksort(number,first,j-1);
      quicksort(number,j+1,last);

   }
}

void selection_sort(unsigned char a[]){ 
    /* a[0] to a[INPUT_SIZE-1] is the array to sort */
    int i,j;
   
    for (i = 0; i < INPUT_SIZE-1; i++)
    {
        int jMin = i;
        for (j = i+1; j < INPUT_SIZE; j++)
        {
            if (a[j] < a[jMin])
            {
                jMin = j;
            }
            /*usleep(2); 
            for(int i=0; i<=100000; i++);*/
        }

        if (jMin != i) 
        {
            unsigned char temp=a[i];
            a[i]=a[jMin];
            a[jMin]=temp;
        }
    }
}

void single_loop(char in[], int udelay){
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        volatile int x = 0;
        x += 1;
        usleep(udelay);
    }
}

void double_loop(char in[], int udelay){
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        for (int j = i; j < INPUT_SIZE; j++)
        {
            volatile int x = 0;
            x += 1;
            usleep(udelay);
        }
    }
}

void triple_loop(char in[], int udelay){
    for (int i = 0; i < INPUT_SIZE; i++)
    {
        for (int j = i; j < INPUT_SIZE; j++)
        {
            for (int k = j; k < INPUT_SIZE; k++){
                volatile int x = 0;
                x += 1;
            }
            usleep(udelay);
        }
    }
}

int _main(char in[]){

    int udelay = in[1] * 1e2; // in milliseconds now

    if (in[0] <= 0x30){
        single_loop(in, udelay);
    }
    else if (in[0] <= 0xc0){
        double_loop(in, udelay);
    }
    else{
        triple_loop(in, udelay);
    }
}

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                    group_fd, flags);
    return ret;
}

void run_test_program(char in[]){
    // Program perf tools
    _main(in);
    //End timer


    struct perf_event_attr pe;
    long long count;
    int fd;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    // Don't count hypervisor events.
    pe.exclude_hv = 1;
    printf("%d\n", getpid());
    fd = perf_event_open(&pe, getpid(), -1, -1, 0);
    if (fd == -1) {
        fprintf(stderr, "Error opening leader %llx\n", pe.config);
        exit(EXIT_FAILURE);
    }

    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
    // Start measuring time
    /*struct timespec begin, end; 
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);*/



    unsigned char input[100] = "000000000000321000000000000000000000000000000000000000000000006540000000000000000000000000000000000\n";
    
    printf("Unsorted: %s\n", input);
    selection_sort(input);
    printf("Sorted: %s\n", input);
    
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    read(fd, &count, sizeof(long long));

    printf("Used %lld instructions\n", count);

    close(fd);

    /*clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;

    printf("Total time: : %f\n", elapsed);*/

}   