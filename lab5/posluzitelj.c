#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <errno.h>
#include "types.h"
#include "queue.h"

#define TIME_TRESHOLD 30
#define MAX_SIZE 128
#define MQ_TYPE 1

StsHeader *handle;

int find_iterations(){
    long iterations = 500000000;
    long last_iteration;
    long dx = 10000;
    time_t start_time;
    time_t end_time;
    struct timeval st, et;
    while(1){

        //Capture time
        

        gettimeofday(&st,NULL);
        for(int i=0; i < iterations; i++){
            asm volatile ("":::"memory");
        }
        gettimeofday(&et,NULL);
        int elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
        elapsed = elapsed / 1000; 

        //printf("Elapsed %d\n", elapsed);
        if(abs(1000 - elapsed) < 100){
            dx = dx / 2;
        }
        if(elapsed >= 900 && elapsed <= 1100){
            printf("Found time: %d\n", elapsed);
            break;
        }
        if(elapsed < 800){
            if(dx < 0) dx = -dx/2;
            dx = dx * 2;
            iterations += dx;
        }else if(elapsed > 1200){
            if(dx > 0) dx = -dx;
            dx = dx/2;
            iterations += dx;
        }

    }
    return iterations;
}

int working_thread(pthread_args_t* args){
    
    if(getenv("SRSVLAB5")==NULL){
        printf("Enviroment variable missing!\n");
        exit(1);
    }
    printf("Starting worker %d\n", args->id);
    int *data;
    //int iterations = find_iterations();
    while(1){
        pthread_mutex_lock(args->mutex);
        //while(!args->cond){
            pthread_cond_wait(args->cond, args->mutex);
        //}

        //pthread_mutex_lock(args->mutex);
        // Read task from queue
        task_t *mtasks[2];
        task_t *mtask;
        if(StsQueue.getSize(handle) >= 2){
            mtasks[1] = StsQueue.pop(handle);
        }
        mtasks[0] = StsQueue.pop(handle);

        if(mtasks[0] == NULL){
            //printf("Error while reading task\n");
            continue;
        }

        if(mtasks[1] != NULL){
            printf("R%d took %d and %d\n", args->id, mtasks[0]->ID, mtasks[1]->ID);
        }

        for(int k = 0; k < 2; k++){
            task_t *mtask = mtasks[k];
            if(mtask == NULL){
                continue;
            }
            // Process the tasks
            // - Read the shared memory data
            //data = mmap(mtask->shm_key, (void*)0, 0);
            char task_shm_string[50];
            sprintf(task_shm_string, "%s-%d", getenv("SRSVLAB5"), mtask->ID);
            int shm_id = shm_open(task_shm_string, O_RDWR | O_CREAT, 0666);
            ftruncate(shm_id, mtask->duration * sizeof(int));
            data = mmap(NULL, mtask->duration * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
            if(data == -1 ){
                printf("Error while getting shared memory segment!\n");
                exit(1);
            }

            for(int i=0; i < mtask->duration; i++){
                int data_elem = data[i];
                // Simulate processing data
                for(int j=0; j < args->iterations; j++){
                    asm volatile ("":::"memory");
                }
                printf("R%d: id:%d obrada podatka: %d (%d/%d)\n", args->id, mtask->ID, data_elem, i+1, mtask->duration);
                
            }

            munmap(data, mtask->duration * sizeof(int));
            shm_unlink(task_shm_string);
            free(mtask);
            pthread_mutex_unlock(args->mutex);

        }

        

    }
    
}

// int msgid, int n, int m, pthread_mutex_t *thread_mutexes
int serving_thread(server_args_t *args){
    msgbuf_t *msgbuf;
    msgbuf = malloc(sizeof(msgbuf_t));
    unsigned long last_task_time = (unsigned) time(NULL);
    int active_taskts = 0;
    printf("Starting server\n");
    while(1){
        unsigned long current_time = (unsigned) time(NULL);
        if(active_taskts > 0 &&  (active_taskts >= args->n || ((current_time - last_task_time) >= TIME_TRESHOLD)  )){
            // If there are more then N jobs in the queue, wake up blocked threads
            printf("P: Waking up workers\n");
            for(int i=0; i<args->n; i++){
                //pthread_mutex_unlock(&args->thread_mutexes[i]);
                pthread_cond_signal(&args->thread_conds[i]);
            }
            active_taskts -= args->n;
            if(active_taskts < 0) active_taskts = 0;
            continue;
        }
    
        

        if(mq_receive(args->msgid, (char*) msgbuf, 2*sizeof(msgbuf_t), NULL) < 0){
            if(errno == EAGAIN){
                continue;
            }
            perror("mq_recv");
        }else{
            printf("MQ: Received task: %d\n", msgbuf->mtask.ID);
            //printf("Pushing task: %d\n", &msgbuf->mtask);
            task_t *new_task = malloc(sizeof(task_t));
            memcpy(new_task, &msgbuf->mtask, sizeof(task_t));
            StsQueue.push(handle, new_task);
            active_taskts++;
            last_task_time = (unsigned) time(NULL);
        }
        
        sleep(1);
        

    }

}


void print_message(int i){
    printf("Message %d\n", i);
}


int main(int argc, char **argv, char **envp){


    int n_working_threads;
    int m;

    

    if(getenv("SRSVLAB5") == NULL){
        printf("Environment variable not set\n");
        exit(1);
    }


    printf("Starting main!\n");

    char env[100];
    strcpy(env, "/"); 
    strcat(env, getenv("SRSVLAB5"));
    char mq_name[100];
    
    pthread_t *threads;
    pthread_mutex_t *thread_mutexes;
    pthread_cond_t *thread_conds;


    //printf("Initializing StsQueue!\n");
    handle = StsQueue.create();

    if (argc == 3){
        n_working_threads = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    
    threads = (pthread_t*) malloc(n_working_threads * sizeof(pthread_t));
    thread_mutexes = (pthread_mutex_t*) malloc(n_working_threads * sizeof(pthread_mutex_t));
    thread_conds = (pthread_cond_t *) malloc(n_working_threads * sizeof(pthread_cond_t));


    if(threads == NULL){
        printf("Malloc failed!\n");
        exit(1);
    }

    
    //printf("Read environment: %s\n", env);
    strcpy(mq_name, env);
    strcat(mq_name, "-mq");

    //key_t key = 0; /* key to be passed to msgget() */ 
    int msqid; /* return value from msgget() */ 
    //msqid = msgget(ftok(mq_name, key), (IPC_CREAT | 0600));
    struct mq_attr attr;  
    attr.mq_flags = 0;  
    attr.mq_maxmsg = 10;  
    attr.mq_msgsize = 33;  
    attr.mq_curmsgs = 0; 
    msqid = mq_open(mq_name, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &attr);

    //printf("MQ: %d\n", msqid);

    pthread_attr_t pthread_attrs[2];
    pthread_attr_init(&pthread_attrs[0]);
    pthread_attr_setinheritsched(&pthread_attrs[0], PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&pthread_attrs[0], SCHED_RR);
    pthread_attr_init(&pthread_attrs[1]);
    pthread_attr_setinheritsched(&pthread_attrs[1], PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&pthread_attrs[1], SCHED_RR);

    struct sched_param sched_params[2];
    sched_params[0].__sched_priority = 60;
    sched_params[1].__sched_priority = 40;
    pthread_attr_setschedparam(&pthread_attrs[0], &sched_params[0]);
    pthread_attr_setschedparam(&pthread_attrs[1], &sched_params[1]);
    


    // Create thread for server
    server_args_t srv_args;
    srv_args.msgid = msqid;
    srv_args.n = n_working_threads;
    srv_args.m = 0;
    srv_args.thread_mutexes = thread_mutexes;
    srv_args.thread_conds = thread_conds;

    pthread_t server_thread;
    pthread_create(&server_thread, &pthread_attrs[0], serving_thread, &srv_args);

    int iterations = find_iterations();

    
    pthread_args_t thread_args[n_working_threads];
    pthread_mutexattr_t thread_attr;
    pthread_mutexattr_init(&thread_attr);
    pthread_mutexattr_settype(&thread_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    // Create worker threads
    for(int i=0; i < n_working_threads; i++){
        // Create tasks for every thread
        pthread_mutex_init(&thread_mutexes[i], &thread_attr);

        pthread_cond_init(&thread_conds[i], NULL);
        
        thread_args[i].id = i + 1;
        thread_args[i].mutex = &thread_mutexes[i];
        thread_args[i].cond = &thread_conds[i];
        thread_args[i].iterations = iterations;
        //pthread_mutex_lock(&thread_mutexes[i]);
        pthread_create(&threads[i], &pthread_attrs[1], working_thread, &thread_args[i]);
        //pthread_create(&threads[i], NULL, working_thread, &thread_args[i]);
    }


    
    //serving_thread(msqid, n_working_threads, 0, thread_mutexes);

    // Wait for all threads to finish
    for(int i=0; i < n_working_threads; i++){
        pthread_join(threads[i], NULL);
    }

    pthread_join(server_thread, NULL); 


    free(threads);
    free(thread_mutexes);
    free(thread_conds);

    return 0;




}