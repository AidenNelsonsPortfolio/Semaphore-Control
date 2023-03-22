/* ********************************************************** *
* CS314-001 Project #2                                        *
*                                                             * 
* Your last-three: 353                                        *
* Your course section #: 314-001                              *
*                                                             *
* Spring 2023                                                 *
*                                                             *
* Author:  Aiden Nelson                                       *
* Date of Last Edit: 3/21/2023                                *
* Description: This project controls multiple processes       * 
* with semaphores and shared memory objects.                  *
* ********************************************************** */
#define NUM_REPEAT 5 // each process repeats 
#define READER_TIME_01 300 // 300ms = 0.3 seconds
#define READER_TIME_02 800 // 800ms = 0.8 seconds
#define WRITER_TIME_01 1600 // 1200ms = 1.2 seconds
#define WRITE_TIME_02 1200 // 1600ms = 1.6 seconds

#define SHM_KEY 8265
#define SEM_KEY1 8765
#define SEM_KEY2 8265
#define SEM_KEY3 9265
#define SEM_KEY4 9565
#define SEM_KEY5 7265

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>  

//Semaphore includes
#include <sys/sem.h>
#include <sys/ipc.h>

//Shared memory includes
#include <sys/shm.h>   

//Struct for shared memory
struct my_mem {
    int reader_count;
    int writer_finish_status[2];
    int writer_waiting_status[2];
    char posted_msg[256];
};

//Struct for semaphores
union semun {
    int val;
    struct semid_ds *buf;
    ushort *arry;
};

//Initialize shared memory

int shmid;
int shmsize;
struct my_mem * shmp;

//Initialize semaphores
union semun mutex1 = {.val = 1};
union semun mutex2 = {.val = 1};
union semun mutex3 = {.val = 1};
union semun counting1 = {.val = 3};
union semun counting2 = {.val = 0};


//Initialize other variables
int mutex1_id = -1;
int mutex2_id = -1;
int mutex3_id = -1;
int counting1_id = -1;
int counting2_id = -1;

int pid = -1;
int mutex1_retval = -1;
int mutex2_retval = -1;
int mutex3_retval = -1; 
int counting1_retval = -1;
int counting2_retval = -1;
struct sembuf mutex1_operations[1];
struct sembuf mutex2_operations[1];
struct sembuf mutex3_operations[1];
struct sembuf counting1_operations[1];
struct sembuf counting2_operations[1];

//Function prototypes

void millisleep(unsigned ms);
void writer(int id);
void reader(int id);


//Main function

int main(void){
    printf("Main has been started...\n");
    fflush(stdout);

    //Create shared memory
    shmsize = sizeof(my_mem);
    if (shmsize <= 0){
        fprintf(stderr, "sizeof error in acquiring the shared memory size. Terminating ..\n");
        exit(0);
    }

    shmid = shmget(SHM_KEY, shmsize, 0666 | IPC_CREAT);
    if (shmid < 0){
        fprintf(stderr, "Failed to create the shared memory. Terminating ..\n");
        exit(0);
    }

    shmp = (struct my_mem *) shmat(shmid, NULL, 0);
    if (shmp == ((struct my_mem *) -1)){
        fprintf(stderr, "Failed to attach the shared memory. Terminating ..\n");
        exit(0);
    }
    shmp->reader_count = 0;
    shmp->writer_finish_status[0] = 1;
    shmp->writer_finish_status[1] = 1;
    shmp->writer_waiting_status[0] = 0;
    shmp->writer_waiting_status[1] = 0;
    sprintf(shmp->posted_msg, "No one has posted a message.");

    //Create semaphores

    //Mutex 1
    mutex1_id = semget(SEM_KEY1, 1, 0666 | IPC_CREAT);
    if (mutex1_id < 0){
        fprintf(stderr, "Failed to create the semaphore. Terminating ..\n");
        exit(0);
    }

    //Initialize mutex 1 to 1
    if (semctl(mutex1_id, 0, SETVAL, mutex1) < 0){
        fprintf(stderr, "Failed to initialize the semaphore. Terminating ..\n");
        exit(0);
    }



    //Mutex 2
    mutex2_id = semget(SEM_KEY2, 1, 0666 | IPC_CREAT);
    if (mutex2_id < 0){
        fprintf(stderr, "Failed to create the semaphore. Terminating ..\n");
        exit(0);
    }

    //Initialize mutex 2 to 1
    if (semctl(mutex2_id, 0, SETVAL, mutex2) < 0){
        fprintf(stderr, "Failed to initialize the semaphore. Terminating ..\n");
        exit(0);
    }


    //Mutex 3
    mutex3_id = semget(SEM_KEY3, 1, 0666 | IPC_CREAT);
    if (mutex3_id < 0){
        fprintf(stderr, "Failed to create the semaphore. Terminating ..\n");
        exit(0);
    }

    //Initialize mutex 3 to 1
    if (semctl(mutex3_id, 0, SETVAL, mutex3) < 0){
        fprintf(stderr, "Failed to initialize the semaphore. Terminating ..\n");
        exit(0);
    }



    //Counting 1
    counting1_id = semget(SEM_KEY4, 1, 0666 | IPC_CREAT);
    if (counting1_id < 0){
        fprintf(stderr, "Failed to create the semaphore. Terminating ..\n");
        exit(0);
    }

    //Initialize counting 1 to 3
    if (semctl(counting1_id, 0, SETVAL, counting1) < 0){
        fprintf(stderr, "Failed to initialize the semaphore. Terminating ..\n");
        exit(0);
    }

    

    //Counting 2
    counting2_id = semget(SEM_KEY5, 1, 0666 | IPC_CREAT);
    if (counting2_id < 0){
        fprintf(stderr, "Failed to create the semaphore. Terminating ..\n");
        exit(0);
    }

    //Initialize counting 2 to 0
    if (semctl(counting2_id, 0, SETVAL, counting2) < 0){
        fprintf(stderr, "Failed to initialize the semaphore. Terminating ..\n");
        exit(0);
    }


    //Create processes

    //Parent is the first reader process.

    //Create the second reader process
    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Failed to create the second reader process. Terminating ..\n");
        exit(0);
    }
    else if (pid == 0){
        reader(2);
        exit(0);
    }


    //Create the third reader process
    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Failed to create the third reader process. Terminating ..\n");
        exit(0);
    }
    else if (pid == 0){
        reader(3);
        exit(0);
    }


    //Create the fourth reader process
    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Failed to create the fourth reader process. Terminating ..\n");
        exit(0);
    }
    else if (pid == 0){
        reader(4);
        exit(0);
    }


    //Create the first writer process
    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Failed to create the first writer process. Terminating ..\n");
        exit(0);
    }
    else if (pid == 0){
        writer(1);
        exit(0);
    }


    //Create the second writer process
    pid = fork();
    if (pid < 0){
        fprintf(stderr, "Failed to create the second writer process. Terminating ..\n");
        exit(0);
    }
    else if (pid == 0){
        writer(2);
        exit(0);
    }


    //Parent calls the first reader process
    reader(1);


    //Parent waits for all child processes to terminate
    //wait on counting2 decremet to 0 (-6)

    //Wait on counting2
    counting2_operations[0].sem_num = 0;
    counting2_operations[0].sem_op = -6;
    counting2_operations[0].sem_flg = 0;

    if (semop(counting2_id, counting2_operations, 1) < 0){
        fprintf(stderr, "Failed to wait on counting2. Terminating ..\n");
        exit(0);
    }

    //Parent removes shared memory and semaphores
    //Remove shared memory, check for errors
    if (shmdt(shmp) < 0){
        fprintf(stderr, "Failed to detach the shared memory. Terminating ..\n");
        exit(0);
    }

    if (shmctl(shmid, IPC_RMID, NULL) < 0){
        fprintf(stderr, "Failed to remove the shared memory. Terminating ..\n");
        exit(0);
    }    

    //Remove semaphores, check for errors
    if (semctl(mutex1_id, 0, IPC_RMID) < 0){
        fprintf(stderr, "Failed to remove mutex1. Terminating ..\n");
        exit(0);
    }
    if (semctl(mutex2_id, 0, IPC_RMID) < 0){
        fprintf(stderr, "Failed to remove mutex2. Terminating ..\n");
        exit(0);
    }
    if (semctl(mutex3_id, 0, IPC_RMID) < 0){
        fprintf(stderr, "Failed to remove mutex3. Terminating ..\n");
        exit(0);
    }
    if (semctl(counting1_id, 0, IPC_RMID) < 0){
        fprintf(stderr, "Failed to remove counting1. Terminating ..\n");
        exit(0);
    }
    if (semctl(counting2_id, 0, IPC_RMID) < 0){
        fprintf(stderr, "Failed to remove counting2. Terminating ..\n");
        exit(0);
    }

    return 0;
}


// millisleep /////////////////////////////////////////////////////////////////
void millisleep(unsigned ms)
{
    usleep(1000*ms);
}


void reader(int id){
    int sleep_time;
    
    //If it is the parent process, add 5 to counting2, b/c it means that 
    //all child processes are made else wait on counting2.
    if (pid != 0){
        printf("All child processes have been made.\nParent process is about to signal for children to start\n");
        fflush(stdout);
        //Add 5 to counting2
        counting2_operations[0].sem_num = 0;
        counting2_operations[0].sem_op = 5;
        counting2_operations[0].sem_flg = 0;
        counting2_retval = semop(counting2_id, counting2_operations, 1);

        if (counting2_retval == -1){
            fprintf(stderr, "Failed to add 5 to counting2 semaphore. Terminating ..\n");
            exit(0);
        }

        else{
            printf("Parent process signaled all children to go.\n");
            fflush(stdout);
        }
    }
    else {
        printf("Waiting on counting2 (Reader %d) \n", id);
        fflush(stdout);
        //Wait on counting2
        counting2_operations[0].sem_num = 0;
        counting2_operations[0].sem_op = -1;
        counting2_operations[0].sem_flg = 0;
        counting2_retval = semop(counting2_id, counting2_operations, 1);

        if (counting2_retval == -1){
            fprintf(stderr, "Failed to wait on counting2 semaphore. Terminating ..\n");
            exit(0);
        }
    }

    printf("Reader %d entering the while loop.\n", id);
    fflush(stdout);

    while(shmp->writer_finish_status[0] == 1 || shmp->writer_finish_status[1] == 1){
        sleep_time = READER_TIME_01;
        bool waited = false;

        millisleep(sleep_time);
        printf("Reader %d likes to read the posted message.\n\n", id);
        fflush(stdout);

        //wait on mutex 3 (mutex3)
        mutex3_operations[0].sem_num = 0;
        mutex3_operations[0].sem_op = -1;
        mutex3_operations[0].sem_flg = 0;
        mutex3_retval = semop(mutex3_id, mutex3_operations, 1);

        if (mutex3_retval == -1){
            fprintf(stderr, "Failed to wait on mutex semaphore. Terminating ..\n");
            exit(0);
        }

        //Check if there is a writer waiting to enter
        if(shmp->writer_waiting_status[0] == 1 || shmp->writer_waiting_status[1] == 1){
            printf("Readers are waiting, because a writer wants to write. \n");
            fflush(stdout);

            mutex1_operations[0].sem_num = 0;
            mutex1_operations[0].sem_op = -1;
            mutex1_operations[0].sem_flg = 0;
            mutex1_retval = semop(mutex1_id, mutex1_operations, 1);

            waited = true;
        }
        else{
            waited = false;
        }

        //Increment reader_count inside of mutex 2 (mutex2)

        mutex2_operations[0].sem_num = 0;
        mutex2_operations[0].sem_op = -1;
        mutex2_operations[0].sem_flg = 0;
        mutex2_retval = semop(mutex2_id, mutex2_operations, 1);

        if (mutex2_retval == -1){
            fprintf(stderr, "Failed to wait on mutex semaphore. Terminating ..\n");
            exit(0);
        }

        shmp->reader_count++;

        //If first reader, let writers know they cannot enter
        //OR, if there is a writer already, it will not let any more readers in
        if (shmp->reader_count == 1 && !waited){

            printf("Readers are waiting on mutex1, there are currently no readers in crit. section. \n\n");
            fflush(stdout);

            //Wait on mutex 1 (mutex1)
            mutex1_operations[0].sem_num = 0;
            mutex1_operations[0].sem_op = -1;
            mutex1_operations[0].sem_flg = 0;
            mutex1_retval = semop(mutex1_id, mutex1_operations, 1);

            if (mutex1_retval == -1){
                fprintf(stderr, "Failed to wait on mutex semaphore. Terminating ..\n");
                exit(0);
            }
        }


        //signal mutex semaphore (mutex2)

        mutex2_operations[0].sem_num = 0;
        mutex2_operations[0].sem_op = 1;
        mutex2_operations[0].sem_flg = 0;
        mutex2_retval = semop(mutex2_id, mutex2_operations, 1);
        
        if (mutex2_retval == -1){
            fprintf(stderr, "Failed to signal mutex semaphore. Terminating ..\n");
            exit(0);
        }



        //signal mutex semaphore (mutex3)

        mutex3_operations[0].sem_num = 0;
        mutex3_operations[0].sem_op = 1;
        mutex3_operations[0].sem_flg = 0;
        mutex3_retval = semop(mutex3_id, mutex3_operations, 1);
        
        if (mutex3_retval == -1){
            fprintf(stderr, "Failed to signal mutex semaphore. Terminating ..\n");
            exit(0);
        }
        ////////////////////////////////////////////////////////////



        //Wait on counting semaphore (counting1)
        counting1_operations[0].sem_num = 0;
        counting1_operations[0].sem_op = -1;
        counting1_operations[0].sem_flg = 0;
        counting1_retval = semop(counting1_id, counting1_operations, 1);

        if (counting1_retval == -1){
            fprintf(stderr, "Failed to wait on counting semaphore. Terminating ..\n");
            exit(0);
        }


        //Start of the critical section
        printf("\nReader %d starts reading the message:\t", id);
        printf("%s\n\n", shmp->posted_msg);
        fflush(stdout);
        
        sleep_time = READER_TIME_02;
        millisleep(sleep_time);
        printf("Reader %d finishes reading the posted message.\n\n", id);
        fflush(stdout);
        //End of the critical section


        //signal counting semaphore (counting1)
        counting1_operations[0].sem_num = 0;
        counting1_operations[0].sem_op = 1;
        counting1_operations[0].sem_flg = 0;
        counting1_retval = semop(counting1_id, counting1_operations, 1);

        if (counting1_retval == -1){
            fprintf(stderr, "Failed to signal counting semaphore. Terminating ..\n");
            exit(0);
        }


        //////////////////////////////////////////////////////////

        //Decrement reader_count inside of mutex 2 (mutex2)

        mutex2_operations[0].sem_num = 0;
        mutex2_operations[0].sem_op = -1;
        mutex2_operations[0].sem_flg = 0;
        mutex2_retval = semop(mutex2_id, mutex2_operations, 1);
        
        if (mutex2_retval == -1){
            fprintf(stderr, "Failed to wait on mutex semaphore. Terminating ..\n");
            exit(0);
        }

        shmp->reader_count--;

        //If last reader, let writers know they can enter
        if (shmp->reader_count == 0){
            printf("There are no more readers in the critical section\n\n");
            //signal mutex semaphore (mutex1)
            mutex1_operations[0].sem_num = 0;
            mutex1_operations[0].sem_op = 1;
            mutex1_operations[0].sem_flg = 0;
            mutex1_retval = semop(mutex1_id, mutex1_operations, 1);

            if (mutex1_retval == -1){
                fprintf(stderr, "Failed to signal mutex semaphore. Terminating ..\n");
                exit(0);
            }
        }

        //signal mutex semaphore (mutex2)

        mutex2_operations[0].sem_num = 0;
        mutex2_operations[0].sem_op = 1;
        mutex2_operations[0].sem_flg = 0;
        mutex2_retval = semop(mutex2_id, mutex2_operations, 1);

        if (mutex2_retval == -1){
            fprintf(stderr, "Failed to signal mutex semaphore. Terminating ..\n");
            exit(0);
        }

        ////////////////////////////////////////////////////////////


    }

    printf("\n*****************\nReader %d is done with while loop \n*****************\n\n", id);
    fflush(stdout);

    //Signal counting2 semaphore
    counting2_operations[0].sem_num = 0;
    counting2_operations[0].sem_op = 1;
    counting2_operations[0].sem_flg = 0;
    counting2_retval = semop(counting2_id, counting2_operations, 1);
    
    if (counting2_retval == -1){
        fprintf(stderr, "Failed to signal counting semaphore. Terminating ..\n");
        exit(0);
    }
    return;
}

void writer(int id){

    int sleep_time;
    char my_message[100];
    char loop_num[NUM_REPEAT/10];
    char res[255];
    sprintf(my_message, "Hello, I am writer %d in loop #", id);
    

    printf("Waiting on counting2 (Writer %d) \n", id);
    fflush(stdout);
    //Wait on counting2 semaphore (parent process will signal this semaphore when child processes are made)
    counting2_operations[0].sem_num = 0;
    counting2_operations[0].sem_op = -1;
    counting2_operations[0].sem_flg = 0;
    counting2_retval = semop(counting2_id, counting2_operations, 1);
    
    if (counting2_retval == -1){
        fprintf(stderr, "Failed to wait on counting2 semaphore. Terminating ..\n");
        exit(0);
    }

    millisleep(id*1000);

    printf("Writer %d enters the for loop \n", id);
    fflush(stdout);
    //Update shared memory message NUM_REPEAT times

    for(int i = 0; i < NUM_REPEAT; i++){
        sleep_time = WRITER_TIME_01;
        millisleep(sleep_time);
        

        shmp-> writer_waiting_status[id-1] = 1; //Set writer status to 1 (waiting to write)
        printf("Writer %d likes to post its message.\n\n", id);
        fflush(stdout);

        //Wait on mutex 1
        mutex1_operations[0].sem_num = 0;
        mutex1_operations[0].sem_op = -1;
        mutex1_operations[0].sem_flg = 0;
        mutex1_retval = semop(mutex1_id, mutex1_operations, 1);

        if (mutex1_retval == -1){
            fprintf(stderr, "Failed to wait on mutex 1. Terminating ..\n");
            exit(0);
        }

        sprintf(res, "%s%d", my_message, i+1);

        //Start of the critical section
        printf("Writer %d starts posting a new message: \n", id);
        printf("%s\n\n", res);
        fflush(stdout);

        strcpy(shmp->posted_msg, res);

        sleep_time = WRITE_TIME_02;
        millisleep(sleep_time);

        printf("Writer %d finishes posting a new message.\n\n", id);
        fflush(stdout);
        //End of the critical section


        //Set writing status to not waiting (0)
        shmp->writer_waiting_status[id-1] = 0;

        //signal mutex 1
        mutex1_operations[0].sem_num = 0;
        mutex1_operations[0].sem_op = 1;
        mutex1_operations[0].sem_flg = 0;
        mutex1_retval = semop(mutex1_id, mutex1_operations, 1);

        if (mutex1_retval == -1){
            fprintf(stderr, "Failed to signal mutex 1. Terminating ..\n");
            exit(0);
        }


    }

    printf("\n*****************\nWriter %d is done with the for loop \n*****************\n", id);
    fflush(stdout);

    //Update writer_status in shared memory
    shmp->writer_finish_status[id-1] = 0;

    //signal counting2 semaphore
    counting2_operations[0].sem_num = 0;
    counting2_operations[0].sem_op = 1;
    counting2_operations[0].sem_flg = 0;
    counting2_retval = semop(counting2_id, counting2_operations, 1);

    if (counting2_retval == -1){
        fprintf(stderr, "Failed to signal counting semaphore. Terminating ..\n");
        exit(0);
    }
    return;
}
