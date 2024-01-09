#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#define READ 0
#define WRITE 1

long long int result_C1, result_C2;
pthread_cond_t condition1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_A = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_B = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_C = PTHREAD_MUTEX_INITIALIZER;

int c1_signal = 0;
int c2_signal = 0;
int c3_signal = 0;

void* c1_task(void* param) {
    //printf("Waiting..");
    pthread_mutex_lock(&lock_A);
    if (c1_signal != 1) {
        pthread_cond_wait(&condition1, &lock_A);
    }
    clock_t start, end;
    start = clock();
    printf("\nChild 1 begins at t = %.10f\n", (double)start / CLOCKS_PER_SEC);
    int* p = (int*)param;
    int n = *p;
    for (int i = 1;i <= n;i++) {
        result_C1 += i;
    }

    end = clock();
    printf("Child 1 ends at t = %.10f\n", (double)end / CLOCKS_PER_SEC);

    clockid_t clk_id;
    struct timespec tspec;
    pthread_t tid = pthread_self();
    if (pthread_getcpuclockid(tid, &clk_id) == 0)
    {
        if (clock_gettime(clk_id, &tspec) != 0)
        {
            perror("clock_gettime():");
        }
        else
        {
            printf("T1 CPU time for tid %lu is %ld seconds, %ld nanoseconds.\n",
                tid, tspec.tv_sec, tspec.tv_nsec);
        }
    }
    else
    {
        printf("pthread_getcpuclockid(): no thread with ID %lu.\n", tid);
    }

    pthread_mutex_unlock(&lock_A);
}

void* c2_task(void* param) {
    pthread_mutex_lock(&lock_B);
    if (c2_signal != 2) {
        pthread_cond_wait(&condition2, &lock_B);
    }
    clock_t start, end;
    start = clock();
    printf("\nChild 2 begins at t = %.10f\n", (double)start / CLOCKS_PER_SEC);
    int* p = (int*)param;
    int n = *p;
    int num;
    int i = 0;
    FILE* file = fopen("sample.txt", "r");
    while (fscanf(file, "%d", &num)) {

        i++;
        if (i <= n) {
            printf("%d\n", num);

        }
        else {
            break;
        }


    }
    end = clock();

    printf("Child 2 ends at t = %.10f\n", (double)end / CLOCKS_PER_SEC);

    clockid_t clk_id;
    struct timespec tspec;
    pthread_t tid = pthread_self();
    if (pthread_getcpuclockid(tid, &clk_id) == 0)
    {
        if (clock_gettime(clk_id, &tspec) != 0)
        {
            perror("clock_gettime():");
        }
        else
        {
            printf("T2 CPU time for tid %lu is %ld seconds, %ld nanoseconds.\n",
                tid, tspec.tv_sec, tspec.tv_nsec);
        }
    }
    else
    {
        printf("pthread_getcpuclockid(): no thread with ID %lu.\n", tid);
    }

    pthread_mutex_unlock(&lock_B);

}

void* c3_task(void* parameters) {
    pthread_mutex_lock(&lock_C);
    if (c3_signal != 3) {
        pthread_cond_wait(&condition3, &lock_C);
    }
    clock_t start, end;
    start = clock();
    printf("\nChild  3 begins at t = %.10f\n", (double)start / CLOCKS_PER_SEC);
    int* p = (int*)parameters;
    int n = *p;
    int num;
    int i = 0;
    FILE* file = fopen("t1.txt", "r");
    while (fscanf(file, "%d", &num)) {

        i++;
        if (i <= n) {
            //printf("%d====%lld\n",num,result_C2);
            result_C2 += num;
        }
        else {
            break;
        }
    }
    end = clock();

    printf("Child 3 ends at t = %.10f\n", (double)end / CLOCKS_PER_SEC);

    clockid_t clk_id;
    struct timespec tspec;
    pthread_t tid = pthread_self();
    if (pthread_getcpuclockid(tid, &clk_id) == 0)
    {
        if (clock_gettime(clk_id, &tspec) != 0)
        {
            perror("clock_gettime():");
        }
        else
        {
            printf("T3 CPU time for tid %lu is %ld seconds, %ld nanoseconds.\n",
                tid, tspec.tv_sec, tspec.tv_nsec);
        }
    }
    else
    {
        printf("pthread_getcpuclockid(): no thread with ID %lu.\n", tid);
    }
    pthread_mutex_unlock(&lock_C);
}

void* monitor_c1(void* param) {
    pthread_mutex_lock(&lock_A);
    int* x = (int*)param;
    int s = *x;

    c1_signal = s;
    pthread_mutex_unlock(&lock_A);
    if (c1_signal == 1) {
        pthread_cond_signal(&condition1);
    }
}

void* monitor_c2(void* param) {
    pthread_mutex_lock(&lock_B);
    int* x = (int*)param;
    int s = *x;

    c2_signal = s;
    pthread_mutex_unlock(&lock_B);
    if (c2_signal == 2) {
        pthread_cond_signal(&condition2);
    }
}

void* monitor_c3(void* param) {
    pthread_mutex_lock(&lock_C);

    int* x = (int*)param;
    int s = *x;

    c3_signal = s;
    pthread_mutex_unlock(&lock_C);
    if (c3_signal == 3) {
        pthread_cond_signal(&condition3);
    }

}

int main() {
    int sch;
    int n1, n2, n3;
    //User Input 

  //Values of N1,N2,N3
    printf("Enter Value Of N1: ");
    scanf("%d", &n1);
    printf("Enter Value Of N2: ");
    scanf("%d", &n2);
    printf("Enter Value Of N3: ");
    scanf("%d", &n3);

    ////User Input for choosing FCFS or RR
    int choice;
    printf("Enter (1) for FCFS and (2) for Round Robin");
    scanf("%d", &choice);
    int quant;
    if (choice == 2) {
        printf("Enter the Value of Time Quantum:");
        scanf("%d", &quant);

        // initlialize the variable name  
        int i, number_of_P, sum = 0, cnt = 0, y, wait_time = 0, tat = 0, at[10], bt[10], temp[10];
        float avg_wait_time, avg_tat;
        printf(" Total number of process in the system: ");
        scanf("%d", &number_of_P);
        y = number_of_P; // Assign the number of process to variable y  

        for (i = 0; i < number_of_P; i++)
        {
            printf("\n Enter the Arrival and Burst time of the Process[%d]\n", i + 1);
            printf(" Arrival time is: \t");  // Accept arrival time  
            scanf("%d", &at[i]);
            printf(" \nBurst time is: \t"); // Accept the Burst time  
            scanf("%d", &bt[i]);
            temp[i] = bt[i]; // store the burst time in temp array  
        }
        // Accept the Time qunat  
        printf("Enter the Time Quantum for the process: \t");
        scanf("%d", &quant);
        // Display the process No, burst time, Turn Around Time and the waiting time  
        printf("\n Process No \t\t Burst Time \t\t tat \t\t Waiting Time ");
        for (sum = 0, i = 0; y != 0; )
        {
            if (temp[i] <= quant && temp[i] > 0) // define the conditions   
            {
                sum = sum + temp[i];
                temp[i] = 0;
                cnt = 1;
            }
            else if (temp[i] > 0)
            {
                temp[i] = temp[i] - quant;
                sum = sum + quant;
            }
            if (temp[i] == 0 && cnt == 1)
            {
                y--; //decrement the process no.  
                printf("\nProcess No[%d] \t\t %d\t\t\t\t %d\t\t\t %d", i + 1, bt[i], sum - at[i], sum - at[i] - bt[i]);
                wait_time = wait_time + sum - at[i] - bt[i];
                tat = tat + sum - at[i];
                cnt = 0;
            }
            if (i == number_of_P - 1)
            {
                i = 0;
            }
            else if (at[i + 1] <= sum)
            {
                i++;
            }
            else
            {
                i = 0;
            }
        }
        // represents the average waiting time and Turn Around time  
        avg_wait_time = wait_time * 1.0 / number_of_P;
        avg_tat = tat * 1.0 / number_of_P;
        printf("\n Average Turn Around Time: \t%f", avg_wait_time);
        printf("\n Average Waiting Time: \t%f", avg_tat);
    }



    pthread_t p1, p2, p3, m1, m2, m3;
    pthread_attr_t at;
    pthread_attr_init(&at);
    int fd[2];
    pipe(fd);
    int shmid = shmget(IPC_PRIVATE, 1024, 0666 | IPC_CREAT);
    int* a, * b, * c, * d;
    int pid = fork();


    if (pid > 0) {// Parent
        a = (int*)shmat(shmid, 0, 0);
        a[0] = 1; // wake C1 thread          
        wait(NULL);
        close(fd[WRITE]);
        long long int c1[1];
        read(fd[READ], c1, sizeof(long long int));
        printf("C1 result recieved = %lld\n", c1[0]);

        int fd1[2];
        pipe(fd1);
        int pid1 = fork();
        if (pid1 > 0)
        {
            // Parent
            a[0] = 2; //wake C2 thread
            wait(NULL);
            int fd2[2];
            pipe(fd2);
            close(fd1[WRITE]);
            char str[100];
            read(fd1[READ], str, 14);
            printf("%s\n", str);
            int pid2 = fork();

            if (pid2 > 0)
            {
                // Parent
                a[0] = 3; // wake C3 thread
                wait(NULL);
                close(fd2[WRITE]);
                long long int e[1];
                read(fd2[READ], e, sizeof(long long int));
                printf("C3 Result Recieved = %lld\n", e[0]);

                shmdt(a);
                shmctl(shmid, IPC_RMID, 0);


            }
            else
            {
                // Child 3
                close(fd2[READ]);
                d = (int*)shmat(shmid, 0, 0);
                printf("%d\n", d[0]);

                pthread_create(&p3, &at, c3_task, &n3);

                pthread_create(&m3, &at, monitor_c3, &d[0]);

                pthread_join(m3, NULL);
                pthread_join(p3, NULL);
                long long int f3[1];
                f3[0] = result_C2;

                printf("C3 writing...\n");
                write(fd2[WRITE], f3, sizeof(long long int));

                shmdt(d);
                exit(0);

            }

        }
        else {//Child 2
                // Read n2 numbers from a txt file and print to console
                // Prints the output along with time of execution
            c = (int*)shmat(shmid, 0, 0);

            pthread_create(&p2, &at, c2_task, &n2);
            pthread_create(&m2, &at, monitor_c2, &c[0]);
            pthread_join(m2, NULL);
            pthread_join(p2, NULL);
            char* msg = "Done Printing";
            write(fd1[WRITE], msg, strlen(msg) + 1);
            shmdt(c);
            exit(0);

        }

    }
    else { // Child 1
    // Sums up n1 numbers
        close(fd[READ]);
        b = (int*)shmat(shmid, 0, 0);
        pthread_create(&p1, &at, c1_task, &n1);
        pthread_create(&m1, &at, monitor_c1, &b[0]);
        pthread_join(m1, NULL);
        pthread_join(p1, NULL);

        //printf("C1 result= %lld\n",result_C1);
        long long int d1[1];
        d1[0] = result_C1;
        double t1[1];
        // printf("c1 writing...\n");
        write(fd[WRITE], d1, sizeof(long long int));
        shmdt(b);
        exit(0);
    }
    return 0;
}