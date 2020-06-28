#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>

struct Context {
    pthread_mutex_t *mutex;
    pthread_cond_t *condition;
    pthread_t thread;
    int* pipes;
    int count;
    uint64_t until;
    uint64_t base;
};

int isPrime (uint64_t number) {
    uint64_t edge = (uint64_t)sqrt(number) + 1;

    if (number <= 1) return 0;
    if (!(number % 2)) return 0;

    for (uint64_t i = 2; i < edge; i++)
        if (number % i == 0) return 0;

    return 1;
}

void* find(void* arg) {
    const struct Context* primeCtx = arg;
    uint64_t i = primeCtx->base;

    pthread_mutex_lock(primeCtx->mutex);
    while (1) {
        if (isPrime(i)) {
            write(primeCtx->pipes[0], &i, sizeof(uint64_t));
            pthread_cond_signal(primeCtx->condition);
            pthread_cond_wait(primeCtx->condition, primeCtx->mutex);
        }
        i++;
    }
    pthread_mutex_unlock(primeCtx->mutex);

    return NULL;
}

int main(void) {
    uint64_t base;
    int count;
    int pipes[2];

    if (scanf("%lld %d", &base, &count) == EOF) return 1;
    if (count <= 0) {
        return 1;
    }
    socketpair(PF_LOCAL, SOCK_STREAM, 0, pipes);
    uint32_t numberOfPrimes = 0;

    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_attr_setguardsize(&attr, 0);

    struct Context primeCtx;
    primeCtx.count = count;
    primeCtx.until = base + count;
    primeCtx.condition = &condition;
    primeCtx.mutex = &mutex;
    primeCtx.base = base;
    primeCtx.pipes = pipes;

    pthread_create(&primeCtx.thread, &attr, find, &primeCtx);

    pthread_mutex_lock(primeCtx.mutex);
    while (numberOfPrimes < count) {
        pthread_cond_wait(primeCtx.condition, primeCtx.mutex);
        uint64_t value;
        read(primeCtx.pipes[1], &value, sizeof(uint64_t));
        numberOfPrimes += 1;
        printf("%lld\n", value);
        pthread_cond_signal(primeCtx.condition);
    }

    pthread_cond_destroy(primeCtx.condition);
    pthread_attr_destroy(&attr);
    pthread_cancel(primeCtx.thread);

    return 0;
}
