#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>


#define KEYSHM 400
#define KEYSHM2 500
#define KEYSHM3 600
#define KEYSHM4 700
#define KEYSEM 800

void sem_signal(int semid, int val) {
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = val;
    semaphore.sem_flg = 1;
    semop(semid, &semaphore, 1);
}

void sem_wait(int semid, int val) {
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = -val;
    semaphore.sem_flg = 1;
    semop(semid, &semaphore, 1);
}



int main(int argc, char**argv){

    FILE *file;
    file = fopen(argv[1],"r");

    int paint_number;
    fscanf(file, "%d", &paint_number);

    int shmid;
    char *ptr;
    shmid = shmget(KEYSHM, paint_number * sizeof(char), 0700|IPC_CREAT);
    ptr = (char *)shmat(shmid, NULL, 0);

    int shmid2;
    int *painting;
    shmid2 = shmget(KEYSHM2, sizeof(int), 0700|IPC_CREAT);
    painting = (int *)shmat(shmid2, NULL, 0);
    *painting = 0;

    int shmid3;
    char *colors;
    shmid3 = shmget(KEYSHM3, 6 * sizeof(char), 0700|IPC_CREAT);
    colors = (char *)shmat(shmid3, NULL, 0);
   *colors = *ptr;

    int shmid4;
    int *index;
    shmid4 = shmget(KEYSHM4, sizeof(int), 0700|IPC_CREAT);
    index = (int *)shmat(shmid4, NULL, 0);
    *index = 1;

    int semid;
    semid = semget(KEYSEM, 1, IPC_CREAT|0700);
    semctl(semid, 0, SETVAL, 0);

    int f, child_id;
    char color;


    FILE *write = fopen("outFile.txt","w");
    int i=0,j=0;
    for(i=0;i<paint_number;i++){
        f = fork();
        fscanf(file, "\n%c", &color);
        *(ptr+i) = color;
        
        bool control = 1; 
        for(j=0;j<6;j++){
            if(*(colors+j) == color) {
                control = 0;
                break;
            }
        }

        if(control){
            *(colors+*index) = color;
            *index += 1;
            sleep(2);
        }

        if(f == 0){
            child_id = 100+i;
            break;
        }   
    }

    if(f == 0){
        while(*(colors+(*painting)) != color );
        sleep(3);
        fprintf(write, "%d %c\n", child_id, color);
        *painting += 1;   
    }
    else{
        sem_wait(semid, paint_number);
    }

}