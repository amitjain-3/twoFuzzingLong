#ifdef __APPLE__
    #include <mach/thread_act.h>
#endif


void some_old_main_stuff(){
    int processorCount = 1; //default 
    pthread_attr_t *affinity_attr = NULL; 

    processorCount = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of logical cores: %d\n", processorCount);


#ifdef __APPLE__
    /*need to figure out how to set affinity, cpu_set_t does not exist. 
      doesnt look like we can set affinity explictly on OSX, 
      but can give hints to kernel which threads do not need to share L2 cache so that they can be scheduled appropriate */
    printf("Still working on this \n");
#elif __linux__
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);
    printf("initialized attribute \n");
#endif
    
    pthread_t threads[processorCount];

    int rc;
    int i;
    for( i = 0; i < processorCount; i++ ) {
        //assign cpu mask here in cpus and use set affinity passing cpu to create attribute, pass attr to pthread_create on each loop (works for linux - need different way for OSX/unix)
#ifdef __linux__
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        affinity_attr = &attr;
#endif
        rc = pthread_create(&threads[i], affinity_attr, add, (void *)i);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
        //usleep(250000); 
    }

    for( i = 0; i < processorCount; i++ ) {
        pthread_join(threads[i], NULL);
    }

    for( i = 0; i < processorCount-3; i++ ) {
#ifdef __linux__
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        affinity_attr = &attr;
#endif
        rc = pthread_create(&threads[i], affinity_attr, rem, NULL);
        if (rc) {
            printf("Error:unable to create thread, %d\n", rc);
            exit(-1);
        }
        //usleep(250000);
    }

    for( i = 0; i < processorCount-3; i++ ) {
        pthread_join(threads[i], NULL);
    }

    queue_print();
    avada_Qdavra();
    
    // Execution time Test
    /*double time_spent = 0.0;
    clock_t begin = clock();
    // do some stuff here
    unsigned char input[100] = "000000000000321000000000000000000000000000000000000000000000006540000000000000000000000000000000000\n";
    //selection_sort(input);
    sleep(3);
    clock_t end = clock();
    // calculate elapsed time by finding difference (end - begin) and
    // dividing the difference by CLOCKS_PER_SEC to convert to seconds
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("The elapsed time is %f seconds \n", time_spent);
    printf("The clocks elapsed is %lu \n", (end-begin));*/

    pthread_mutex_destroy(&qlock);
    pthread_exit(NULL);

    return 0;
}

Write mutate func
Store interesting inputs in a queue
Find a way to run external programs
Measure the rntime
Get the appropriate seed input

Funcs that we need
1. Queue operations: get, put, remove, size, 
2. Mutate operations: bit flip, delete/add element, 
3. Run test program: get runtime, get crash status, etc, maybe coverage later
4. Define code structure: data structure operations, branch behaviour 