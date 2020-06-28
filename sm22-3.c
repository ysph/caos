#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>

pthread_mutex_t processingMutex;

void* scan_value(void* arg) {
    int value = 0, result = 0;

    pthread_mutex_lock(&processingMutex);
    while (scanf("%d", &value) != EOF) {
        result += value;
        sched_yield();
    }
    pthread_mutex_unlock(&processingMutex);

    return (void*)result;
}

int main(int argc, const char **argv) {
    int result = 0;
    int i;
    uint64_t num_of_threads;

    if (argv[1] == NULL) return 1;
    num_of_threads = strtol(argv[1], NULL, 10);
    if (num_of_threads <= 0) return 1;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_attr_setguardsize(&attr, 0);

    pthread_t* threads = calloc(num_of_threads, (sizeof(pthread_t)));

    for (i = 0; i < num_of_threads; ++i) {
        pthread_create(threads + i, &attr, &scan_value, NULL);
    }

    pthread_attr_destroy(&attr);

    for (i = 0; i < num_of_threads; ++i) {
        void* retval = 0;
        pthread_join(*(threads + i), &retval);
        result += (int)retval;
    }

    printf("%d\n", result);

    return 0;
}
