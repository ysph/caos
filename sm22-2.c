#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_ARRAY 100

int counter = 0;

void* print(void* arg) {
    printf("%d\n", *(int*)arg);

    return NULL;
}

int main(void) {
    pthread_t* tid;
    int i;
    int number;
    int arr[MAX_ARRAY];

    while ((scanf("%d", &number) == 1) && (counter < MAX_ARRAY)) {
        arr[counter] = number;
        counter++;
    }

    tid = (pthread_t*)malloc(sizeof(pthread_t) * counter);

    for (i = counter - 1; i >= 0; i--) {
        pthread_create(&tid[i], NULL, print, (void*)&arr[i]);
        sleep(0.1);
    }

    for (i = counter - 1; i >= 0; i--) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
