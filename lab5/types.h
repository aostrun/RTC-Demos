


typedef struct {
    int ID;
    unsigned int duration;
    int shm_key; 
} task_t;

typedef struct {
    long mtype;
    task_t mtask;
} msgbuf_t;

typedef struct {
    pthread_mutex_t mutex;
    int last_id;
} gendata_t;

typedef struct {
    int id;
    int iterations;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
} pthread_args_t;

typedef struct{
    int msgid; 
    int n; 
    int m; 
    pthread_mutex_t *thread_mutexes;
    pthread_cond_t *thread_conds;
} server_args_t;