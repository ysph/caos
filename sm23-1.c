#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMBER_OF_THREADS 3
#define NUMBER_OF_OPERATIONS 1000000

double array[NUMBER_OF_THREADS] = { 0 };
pthread_mutex_t processingMutex;

void* print(void* arg) {
    int turn = *(int*)arg;
    int i;

    pthread_mutex_lock(&processingMutex);

    for (i = 0; i < NUMBER_OF_OPERATIONS; i++) {
        if (turn == 0) {
            array[0] += 100;
            array[1] -= 101;
        } else if (turn == 1) {
            array[1] += 200;
            array[2] -= 201;
        } else if (turn == 2) {
            array[2] += 300;
            array[0] -= 301;
        }
    }

    pthread_mutex_unlock(&processingMutex);

    return NULL;
}

int main(void) {
    pthread_t* tid;
    volatile int i;
    int* arr;

    tid = (pthread_t*)malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);
    arr = (int*)malloc(sizeof(int) * NUMBER_OF_THREADS);

    for (i = 0; i < NUMBER_OF_THREADS; i++) {
        arr[i] = i;
        pthread_create(&tid[i], NULL, print, (void*)&arr[i]);
    }

    for (i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(tid[i], NULL);
    }

    for (i = 0; i < NUMBER_OF_THREADS; i++) {
        printf("%.10g\n", array[i]);
    }

    free(tid);
    free(arr);

    return 0;
}
