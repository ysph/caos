#include <time.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <limits.h>

static int32_t max_number = 999999999;
static int32_t nsec_in_sec = 1000000;
static int32_t max_timestamp = 2147483647;

int main(void) {
    struct timespec end;
    struct timeval cur;
    long int secs;
    int sfd;

    scanf("%ld %ld", &end.tv_sec, &end.tv_nsec);
    if (end.tv_sec >= max_timestamp) return -1;
    if ((end.tv_nsec > max_number) && (end.tv_nsec < 0)) return -1;

    secs = end.tv_sec;
    secs *= 1000;
    secs += ((end.tv_nsec + 500000) / nsec_in_sec);
    gettimeofday(&cur, NULL);
    long long curM = cur.tv_sec * 1000LL + (cur.tv_usec + 500) / 1000;

    if (curM >= secs) return 0;
    if (cur.tv_sec > end.tv_sec) return 0;
    if ((cur.tv_sec == end.tv_sec) && (cur.tv_usec >= end.tv_nsec)) return 0;

    secs -= curM;

    sigset_t mask;
    if (sigemptyset(&mask)) return -1;
    if (sigaddset(&mask, SIGALRM)) return -1;
    if (sigprocmask(SIG_BLOCK, &mask, NULL)) return -1;

    sfd = signalfd(-1, &mask, 0);
    struct signalfd_siginfo si;

    struct itimerval to = {};
    to.it_value.tv_sec = secs / 1000;
    to.it_value.tv_usec = (secs % 1000) * 1000;
    setitimer(ITIMER_REAL, &to, NULL);

    if (read(sfd, &si, sizeof(si)) < sizeof(si)) return 1;

    return 0;
}
