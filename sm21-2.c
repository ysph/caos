#include <time.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <limits.h>
#include <stdlib.h>

volatile uint32_t last = 0;
volatile uint32_t count = 0;
time_t microsec_in_sec = 100000;

int isPrime(uint32_t x){
    if (x <= 1) return 0;

    uint32_t i;
    for(i = 2; i * i <= x; ++i)
        if (!(x % i)) return 0;

    return 1;
}

void action(int arg) {
    (void)arg;

    printf("%u\n", last);
    fflush(stdout);

    count += 1;
    if (count == 8)
        exit(0);
}

int main(void) {
    signal(SIGVTALRM, action);
    uint32_t low, high;

    scanf("%u %u", &low, &high);
    if (low >= high) return -1;
    if (high >= UINT_MAX) return -1;

    struct sigaction sa = {
        .sa_handler = action,
        .sa_flags = SA_RESTART
    };

    if (sigemptyset(&sa.sa_mask)) return -1;
    if (sigaction(SIGVTALRM, &sa, NULL)) {
        fprintf(stderr, "Error\n");
        return -1;
    }

    struct itimerval ival = {};

    ival.it_value.tv_sec = 0;
    ival.it_interval.tv_sec = 0;

    ival.it_value.tv_usec = microsec_in_sec;
    ival.it_interval.tv_usec = microsec_in_sec;

    setitimer(ITIMER_VIRTUAL, &ival, NULL);

    for (uint32_t i = low; i < high; ++i)
        last = isPrime(i) ? i : last;

    if (count != 8) {
        printf("-1\n");
        fflush(stdout);
    }

    return 0;
}

/*
    if (WIFEXITED(status))
        retval = (WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        retval = (1024 + WTERMSIG(status));
    else if (WIFSTOPPED(status))
        retval = (1024 + WSTOPSIG(status));
    else if (WIFCONTINUED(status))
        retval = (1024 + SIGCONT);*/
