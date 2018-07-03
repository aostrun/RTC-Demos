#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/fcntl.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include "types.h"


#define GEN_SHM_SIZE 1024



int main(int argc, char **argv, char **envp){

    int n_tasks;
    int m;

    int gen_shm_id;

    task_t *tasks;

    if(getenv("SRSVLAB5") == NULL){
        printf("Environment variable not set!\n");
        exit(1  );
    }   

    printf("Starting!\n");

    char env[100];
    strcpy(env, "/"); 
    strcat(env, getenv("SRSVLAB5"));
    char shm_gen_name[100];
    
    if (argc == 3){
        n_tasks = atoi(argv[1]);
        m = atoi(argv[2]);
    }

    char mq_name[100];
    printf("Read environment: %s\n", env);
    strcpy(mq_name, env);
    strcat(mq_name, "-mq");


    // Get message queue key and id
    int msqid; /* return value from msgget() */ 
    //msqid = msgget(ftok(mq_name, msq_key), (IPC_CREAT | 0600));
    struct mq_attr attr;  
    attr.mq_flags = 0;  
    attr.mq_maxmsg = 10;  
    attr.mq_msgsize = 33;  
    attr.mq_curmsgs = 0;  
    msqid = mq_open(mq_name, O_RDWR | O_CREAT, 0666, &attr);
    if(msqid < 0){
        perror("mqueue");
        exit(1);
    }

    // Get or create the shared memory segment
    strcpy(shm_gen_name, env);
    strcat(shm_gen_name, "-gen");
    //gen_shm_key = ftok(shm_gen_name ,1);
    

    gen_shm_id = shm_open(shm_gen_name, O_RDWR, 0777);

    gendata_t *gendata;
    if(gen_shm_id == -1 && errno == ENOENT){
        // Shared memory segment doesn't exists yet, create it
        printf("G: Creating new shared memory segment!\n");
        gen_shm_id = shm_open(shm_gen_name, O_RDWR | O_CREAT, 0777);
        ftruncate(gen_shm_id, sizeof(gendata_t));
        gendata = mmap(NULL, sizeof(gendata_t), PROT_READ | PROT_WRITE, MAP_SHARED, gen_shm_id, 0);
        pthread_mutex_init(&(gendata->mutex), NULL);
        gendata->last_id = 0;
    }else{
        gendata = mmap(NULL, sizeof(gendata_t), PROT_READ | PROT_WRITE, MAP_SHARED, gen_shm_id, 0);
    }
    if(gendata < 0){
        printf("mmap error\n");
        perror("mmap");
        exit(1);
    }
    // Lock the mutex
    pthread_mutex_lock(&(gendata->mutex));
    

    // Create n_tasks and their data
    tasks = malloc( n_tasks * sizeof(task_t) );
    for(int i=0; i < n_tasks; i++){
        int random_duration = (rand() % (m - 2)) + 1 ;
        tasks[i].ID = gendata->last_id + 1 + i;
        tasks[i].duration = random_duration;
        char task_shm_string[100];
        sprintf(task_shm_string, "%s-%d", env, tasks[i].ID);
        //key_t task_shm_key = ftok(task_shm_string, 1);
        int shm_id = shm_open(task_shm_string, O_RDWR | O_CREAT, 0666);
        ftruncate(shm_id, tasks[i].duration * sizeof(int));
        //int shm_id = shmget(task_shm_key, GEN_SHM_SIZE, 0666|IPC_CREAT);
        tasks[i].shm_key = shm_id;

        // Create random processing data for the task
        int *task_data = (int*) mmap(NULL, tasks[i].duration * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
        //int *task_data = (int*) shmat(shm_id, (void*)0, 0);
        for(int j=0; j < random_duration; j++){
            task_data[j] = rand();
        }

        msgbuf_t msqbuf;
        msqbuf.mtask = tasks[i];
        msqbuf.mtype = 1;
        // Send task to message queue
        
        if (mq_send(msqid, (const char*) &msqbuf, sizeof(msgbuf_t), 0) < 0) {
            printf ("%d, %ld\n", msqid, msqbuf.mtype);
            perror("msgsnd");
            exit(1);
        }
        printf("G: Generated task: %d\n", tasks[i].ID);


    }
    gendata->last_id = gendata->last_id + n_tasks;

    pthread_mutex_unlock(&(gendata->mutex));


}