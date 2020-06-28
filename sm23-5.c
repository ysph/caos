#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>

struct FairRwLock
{
    int i;
};

typedef struct FairRwLock fair_rwlock_t;

#define FAIR_RWLOCK_INITIALIZER {0}

fair_rwlock_t marw = FAIR_RWLOCK_INITIALIZER;

void fair_rwlock_init(fair_rwlock_t *prw);
void fair_rwlock_destroy(fair_rwlock_t *prw);
void fair_rwlock_wrlock(fair_rwlock_t *prw);
void fair_rwlock_rdlock(fair_rwlock_t *prw);
void fair_rwlock_unlock(fair_rwlock_t *prw);

unsigned int iter;
sem_t accessM, readresM, orderM;
unsigned int readers = 0;

void *reader(void *arg) {
    (void)arg;
    int i, rest;
    for(i = 0; i < iter; i++) {
        sem_wait(&orderM);
        sem_wait(&readresM);
        if (readers == 0) sem_wait(&accessM);
        readers++;
        sem_post(&orderM);
        sem_post(&readresM);

        rest = 1 + rand() % 4;
        sleep(rest);
        sem_wait(&readresM);
        readers--;
        if (readers == 0) sem_post(&accessM);
        sem_post(&readresM);
    }
    return(NULL);
}

void *writer(void *arg) {
    (void)arg;
    int i, rest;
    for(i = 0; i < iter; i++) {
        sem_wait(&orderM);
        sem_wait(&accessM);
        sem_post(&orderM);

        rest = 1 + rand() % 4;
        sleep(rest);
        sem_post(&accessM);
    }
    return(NULL);
}
