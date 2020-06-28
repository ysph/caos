#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
  
#define NUMBER_OF_THREADS 10

int cnt = 0; 

void* print(void* arg) { 
    int turn = *(int*)arg; 

    printf("%d\n", turn); 

    if (cnt < NUMBER_OF_THREADS - 1) { 
        cnt++; 
    } else { 
        cnt = 0; 
    } 
  
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
        sleep(0.1);
    }

    for (i = 0; i < NUMBER_OF_THREADS; i++) { 
        pthread_join(tid[i], NULL); 
    } 
  
    return 0; 
} 
