#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

struct thread_d {
    char team;
};

typedef struct semaphore {
    pthread_mutex_t s_lock;
    pthread_cond_t cond;
    int s_value;
} sem_t;

int val_T, val_O;
sem_t sem_A, sem_B;
sem_t *sem_T, *sem_O;

void sem_init(sem_t *sem, int val) {
    pthread_mutex_init(&sem->s_lock, NULL); 
    pthread_cond_init(&sem->cond, NULL);
    sem->s_value = val;
}

void sem_getvalue(sem_t *sem, int*val){
    *val = sem->s_value;
}

void sem_post(sem_t *sem) {
    pthread_mutex_lock(&sem->s_lock);
    sem->s_value++;
    pthread_cond_signal(&sem->cond); 
    pthread_mutex_unlock(&sem->s_lock);
}

void sem_setvalue(sem_t *sem, int val){
    for (int i = 0; i < val; i++){
        sem_post(sem);
    }
}

void sem_wait(sem_t *sem) {
    pthread_mutex_lock(&sem->s_lock);
    sem->s_value--;
    pthread_mutex_unlock(&lock);    
    if (sem->s_value < 0) {
        pthread_cond_wait(&sem->cond, &sem->s_lock);
    }
    pthread_mutex_unlock(&sem->s_lock);
}


void* thread_func(void * argc) {
    bool driver = false;
    struct thread_d * args = (struct thread_d*) argc;

    pthread_mutex_lock(&lock);
    printf("Thread ID: %ld, Team: %c, I am looking for a car\n", pthread_self(), args->team);

	if (args->team == 'A') {
		sem_T = &sem_A;
		sem_O = &sem_B;
	} else {
		sem_T = &sem_B;
		sem_O = &sem_A;
	}    

    sem_getvalue(sem_T, &val_T);
    sem_getvalue(sem_O, &val_O);

    if(val_T < 0 && val_O < -1) {
        driver = true; sem_post(sem_T);
        sem_setvalue(sem_O, 2); 
    } else if(val_T < -2) {
		driver = true; 
        sem_setvalue(sem_T, 3); 
    } else {
        sem_wait(sem_T);
    }

    printf("Thread ID: %ld, Team: %c, I have found a spot in a car\n", pthread_self(), args->team);
    pthread_barrier_wait(&barrier);
    if(driver) {
        printf("Thread ID: %ld, Team: %c, I am the captain and driving the car\n", pthread_self(), args->team);
		pthread_barrier_destroy(&barrier); 
	    pthread_barrier_init(&barrier, NULL, 4); 
	    pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char* argv[]){
    int numA = atoi(argv[1]);
    int numB = atoi(argv[2]);
    int total = numA + numB;

    if (numA % 2 != 0 || numB % 2 != 0 || total % 4 != 0){
        printf("The main terminates\n");
        return 1;
    }

    pthread_mutex_init(&lock, NULL);
    pthread_barrier_init(&barrier, NULL, 4);

    char c_list[total];
    pthread_t th[total];
    struct thread_d s_list[total];

    srand(time(NULL));

    for (int i = 0; i < numA; i++) {
        c_list[i] = 'A'; 
    }
    for (int i = numA; i < total; i++){
        c_list[i] = 'B'; 
    }

    for (int i = 0; i < total; i++){
        int idx;
        do{
            idx = rand() % total;
        } while(c_list[idx] == '0');

        s_list[i].team = c_list[idx];
        c_list[idx] = '0';
    }

    sem_init(&sem_A, 0);
    sem_init(&sem_B, 0);

    for (int i = 0; i < total; i++){
        if (pthread_create(&th[i], NULL, thread_func, &s_list[i]) != 0) {
            perror("Failed to create thread\n");
            return -1;
        }
    }
    
    for (int i = 0; i < total; i++){
        if (pthread_join(th[i],NULL) != 0) {
            perror("Failed to join thread\n");
            return -1;
        }
    }

    pthread_barrier_destroy(&barrier); 
    printf("The main terminates\n");
    return 0;
}