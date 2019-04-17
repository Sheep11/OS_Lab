#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include<memory.h>
#include <time.h>

#define SEM_MUTEX 1
#define SEM_FULL 2
#define SEM_EMPTY 3
#define SHM_KEY 76
#define PRODUCER_NUM 2
#define CONSUMER_NUM 4

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

//shared memory struct
typedef struct {
    int cursor;
    int buffer[3];
}buffer_t;

int P(int semid)
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;

    return semop(semid, &op, 1);
}

int V(int semid)
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;

    return semop(semid, &op, 1);
}

void show_buffer(buffer_t *pbuffer){
    printf("now buffer: ");
    for(int i=0;i<pbuffer->cursor;i++)
        printf("%d ",pbuffer->buffer[i]);
    printf("\n\n");
}

void show_time(){
    time_t now;
    time(&now);
    printf("%s",asctime(gmtime(&now)));
}

//create a sem with value
//key: key of semaphore set
//value: value of semaphore
int init_sem(int key, int value)
{
    int semid = semget(key, 1, IPC_CREAT | 0600);
    printf("new semid:%d\n", semid);

    union semun sem_val;
    sem_val.val = value;
    semctl(semid, 0, SETVAL, sem_val);

    return semid;
}

int main()
{
    //create semaphore and init
    int mutex = init_sem(SEM_MUTEX, 1);
    int full = init_sem(SEM_FULL, 0);
    int empty = init_sem(SEM_EMPTY, 3);

    int shmid = shmget(SHM_KEY, 16, 0777 | IPC_CREAT);
    printf("shmid:%d\n", shmid);

    //create shared memory and init buffer
    buffer_t *pbuffer = (buffer_t *)shmat(shmid, 0, 0);
    memset(pbuffer,0,sizeof(buffer_t));
    shmdt(pbuffer);

    //create PRODUCER_NUM(2) producer
    for (int i = 0; i < PRODUCER_NUM; i++)
        if (fork() == 0)
        { //producer
            printf("pid:%d, producer:%d \n", getpid(), i);

            //get shared memory
            buffer_t *pbuffer = (buffer_t *)shmat(shmid, 0, 0);

            srand(getpid());
            for (int j = 0; j < 4; j++)
            {
                sleep(rand()%3);

                P(empty);
                P(mutex);

                int cursor= pbuffer->cursor;
                pbuffer->cursor++;
                pbuffer->buffer[cursor]=rand()%100;

                show_time();
                printf(": producer(%d), put %d at %d---", i, pbuffer->buffer[cursor], cursor);
                show_buffer(pbuffer);

                V(mutex);
                V(full);
            }
            shmdt(pbuffer);
            exit(0);
        }

    //create CUNSUMER_NUM(4) consumer
    for (int i = 0; i < CONSUMER_NUM; i++)
        if (fork() == 0)
        { //consumer
            printf("pid:%d, consumer:%d \n", getpid(), i);

            //get shared memory
            buffer_t *pbuffer = (buffer_t *)shmat(shmid, 0, 0);

            srand(getpid());
            for (int j = 0; j < 2; j++)
            {
                sleep(rand()%3);

                P(full);
                P(mutex);

                int cursor= pbuffer->cursor-1;
                pbuffer->cursor--;

                show_time();
                printf(":consumer(%d), get %d at %d---", i, pbuffer->buffer[cursor], cursor);
                show_buffer(pbuffer);

                V(mutex);
                V(empty);
            }
            shmdt(pbuffer);
            exit(0);
        }
    
    //wait all child process
    int pid;
    while(pid=wait(NULL))
        if(pid>=0)  printf("pid:%d exit\n",pid);
        else    break;

    //remove shared memory
    if(shmctl(shmid,IPC_RMID,NULL)==0)
        printf("remove shared memory\n");
    return 0;
}