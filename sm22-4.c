#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>

int* couples;
int count;

void* pong(void *arg) {
    int id = *(int*)arg;
    int t = 0;
    int32_t next = 0;

    while (1) {
        read(couples[(id << 1)], &t, sizeof(t));

        if (scanf("%d", &next) == 1) {
            printf("%d %d\n", id, next);
            fflush(stdout);

            next %= count;
            if (next < 0) next += count;

            write(couples[(next << 1) + 1], &next, sizeof(next));
            fflush(stdout);
        } else {
            next = (id + 1) % count;
            write(couples[(next << 1) + 1], &next, sizeof(next));
            fflush(stdout);
            break;
        }
    }

    return NULL;
}

int main(int argc, char ** argv) {
    if (argc == 2)
        count = atoi(argv[1]);
    else
        return EXIT_FAILURE;

    if (count < 1 && count > 100) return EXIT_FAILURE;

    pthread_t threads[count];
    int thread_ids[count];

    couples = calloc(count << 1, sizeof(int));

    for (int i = 0; i < count; i++) {
        pipe(&(couples[i << 1]));
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_attr_setguardsize(&attr, 0);

    for (int i = 0; i < count; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], &attr, pong, &thread_ids[i]);
    }

    int t = 0;
    write(couples[1], &t, sizeof(t));

    for (int i = 0; i < count; i++)
        pthread_join(threads[i], NULL);

    for (int i = 0; i < count; i++) {
        close(couples[i << 1]);
        close(couples[(i << 1) + 1]);
    }

    pthread_attr_destroy(&attr);
    free(couples);

    return EXIT_SUCCESS;
}
