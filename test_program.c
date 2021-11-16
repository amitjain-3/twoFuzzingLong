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

// int m(){
//     int num = 0;
//     num = num*10 + 0; // nuum = 0
//     if (this){
//         //...
//         num = num*10 + 1; // 1
//     }
//     else{
//         num = num*10 + 2;
//     }

//     num = num*10 + 3; // 13

//     return 1;
// }
void selection_sort(unsigned char a[]){ 
    /* a[0] to a[INPUT_SIZE-1] is the array to sort */
    int i,j;
    /* advance the position through the entire array */
    /*   (could do i < INPUT_SIZE-1 because single element is also min element) */
    for (i = 0; i < INPUT_SIZE-1; i++)
    {
        /* find the min element in the unsorted a[i .. INPUT_SIZE-1] */

        /* assume the min is the first element */
        int jMin = i;
        /* test against elements after i to find the smallest */
        for (j = i+1; j < INPUT_SIZE; j++)
        {
            /* if this element is less, then it is the new minimum */
            if (a[j] < a[jMin])
            {
                /* found new minimum; remember its index */
                jMin = j;
            }
            /*usleep(2); 
            for(int i=0; i<=100000; i++);*/
        }

        if (jMin != i) 
        {
            // swap(a[i], a[jMin]);
            unsigned char temp=a[i];
            a[i]=a[jMin];
            a[jMin]=temp;
        }
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



int main(int argc, char ** argv){
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

