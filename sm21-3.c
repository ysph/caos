#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/eventfd.h>

char *S;
int *counter;

int fd[2];

static int max_buffer = 4096;

void increment(int j, int K) {
    printf("%d %s\n", j, S);
    fflush(stdout);

    int len = strlen(S);
    if (len >= max_buffer) exit(-1);
    int carry = 1;

    for (int i = 0; i < len; ++i) {
        S[i] += carry;

        if (S[i] != '0' + K)
            carry = 0;
        else {
            S[i] = '0';
            carry = 1;
        }
    }

    if (carry) {
        S[len] = '1';
        S[len + 1] = '\0';
    }

    *counter += 1;

    return;
}


int main(int argc, const char **argv) {
    int N, K;
    int process[2] = { 1, 2 };
    void *data;

    if (argc == 4) {
        N = atoi(argv[1]);
        K = atoi(argv[2]);
    } else {
        return 1;
    }

    if (N < 0) return 1;
    if (K < 2 && K > 10) return 1;
    if (!(strlen(argv[3]))) return 1;

    data = mmap(NULL, max_buffer * sizeof(char) + sizeof(int),
            PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

    counter = data;
    *counter = 1;

    S = data + sizeof(int);

    strcpy(S, argv[3]);

    fd[0] = eventfd(0, EFD_SEMAPHORE);
    fd[1] = eventfd(0, EFD_SEMAPHORE);
    if (fd[0] == -1 || fd[1] == -1) return -1;

    if (N) {
        if (!fork()) {
            uint64_t t = 1;

            increment(process[0], K);
            write(fd[1], &t, sizeof(t));

            while (*counter < N) {
                read(fd[0], &t, sizeof(t));
                increment(process[0], K);
                write(fd[1], &t, sizeof(t));
            }
            exit(0);
        }

        if (!fork()) {
            uint64_t t;

            while (*counter < N) {
                read(fd[1], &t, sizeof(t));
                increment(process[1], K);
                write(fd[0], &t, sizeof(t));
            }

            exit(0);
        }
    }

    waitpid(-1, NULL, 0);
    waitpid(-1, NULL, 0);

    printf("Done\n");

    munmap(data, max_buffer * sizeof(char) + sizeof(int));

    return 0;
}
