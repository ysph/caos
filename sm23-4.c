#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

struct Parameters {
    int iterations;
    int32_t index1, index2;

    double sum1, sum2;
    double* accounts;

    pthread_mutex_t* mutexes;
};

void* process(void *arg) {
    struct Parameters* param = (struct Parameters*)arg;
    int until = param->iterations;
    int index1 = param->index1;
    int index2 = param->index2;

    for (int i = 0; i < until; i++) {
        pthread_mutex_lock(&param->mutexes[index1]);
        pthread_mutex_lock(&param->mutexes[index2]);
        param->accounts[index1] += param->sum1;
        param->accounts[index2] += param->sum2;
        pthread_mutex_unlock(&param->mutexes[index1]);
        pthread_mutex_unlock(&param->mutexes[index2]);
    }

    return NULL;
}

int main(void) {
    int32_t acc_count, thr_count;

    scanf("%d %d", &acc_count, &thr_count);
    if (acc_count < 1 || thr_count < 1) return EXIT_FAILURE;

    struct Parameters params[thr_count];
    pthread_t threads[thr_count];

    double accounts[acc_count];
    pthread_mutex_t mutexes[acc_count];

    for (int i = 0; i < acc_count; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
        accounts[i] = 0;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_attr_setguardsize(&attr, 0);

    for (int i = 0; i < thr_count; i++) {
        scanf("%d %d %lg %d %lg",  &params[i].iterations, &params[i].index1,
                                   &params[i].sum1, &params[i].index2, &params[i].sum2);
        if (params[i].iterations < 0)
            return EXIT_FAILURE;
        if ((params[i].index1 < 0) && (params[i].index1 >= acc_count))
            return EXIT_FAILURE;
        if ((params[i].index2 < 0) && (params[i].index2 >= acc_count))
            return EXIT_FAILURE;

        if (params[i].index1 > params[i].index2) {
            params[i].index1 += params[i].index2;
            params[i].index2 = params[i].index1 - params[i].index2;
            params[i].index1 -= params[i].index2;

            double temp = params[i].sum1;
            params[i].sum1 = params[i].sum2;
            params[i].sum2 = temp;
        }

        params[i].mutexes = mutexes;
        params[i].accounts = accounts;

        pthread_create(&threads[i], &attr, process, &params[i]);
    }
    pthread_attr_destroy(&attr);

    for (int i = 0; i < thr_count; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < acc_count; i++) {
        pthread_mutex_destroy(&mutexes[i]);
        printf("%.10g\n", accounts[i]);
    }

    return EXIT_SUCCESS;
}
