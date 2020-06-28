#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <malloc.h>

static pid_t pid;

size_t length;
char* readpipe(int fd) {
    int n, nr, k;
    char *buf;
    n = 0, k = 0;
    buf = NULL;

    int desc = 0;

    while (!feof(stdin)) {
        if (desc > 11) break;
        if (k >= n) {
            if (n == 0)
                n = 1 << 10;
            else
                n <<= 1;

            buf = realloc(buf, n);
            if (buf == NULL) {
                perror("readpipe");
                exit(-1);
            }
        }
        printf("FFFFFFFFFFFFFFf%dFFFFFFFFFFFf\n", 1<<12);
		printf("%ld----------%d-%d=%d----------------\n",length, n,k, n-k);
        nr = read(fd, &buf[k], n - k);
        if (nr < 0) {
            perror("readpipe, read");
            break;
        }
        else if (nr == 0)
            break;
        k += nr;
		printf("!! nr=%d, k=%d\n", nr, k);
        desc++;
    }
    buf[k] = '\0';
	printf("%d___BUF=%d___", k, strlen(buf));
    return buf;
}

void writepipe(int fd, const char* s, size_t n) {
    int nw;
    int k = 0;
    while (n > 0) {
        nw = write(fd, &s[k], n);
        printf("SYKA==%d\n",nw);
        if (nw < 0) {
            perror("writepipe, write");
            break;
        } else if (nw == 0) {
            break;
        }

        k += nw;
        n -= nw;
    }

    return;
}

static void alarm_signal(int signal_number) {
    kill(pid, SIGKILL);
    return;
}

int malarm(int msec) {
    double time;
    time_t sec;
    suseconds_t usec;
    struct itimerval timer;

    time = msec / 1000.0;
    sec = time;
    usec = (time - sec) * 1E6;
    timer.it_value.tv_sec = sec;
    timer.it_value.tv_usec = usec;
    timer.it_interval.tv_sec = sec;
    timer.it_interval.tv_usec = usec;

    if (setitimer(ITIMER_REAL, &timer, NULL)) {
        perror("setitimer");
        exit(-1);
    }
    return ((int)(time * 1000));
}

int run(const char* cmd, const char* input, char** poutput, char** perr, int timeout) {
    int pipein[2], pipeout[2], pipeerr[2];
    int retval, status;
	char *s = "";

    struct sigaction sa;
    length = strnlen(input, (1 << 12));

    if (pipe2(pipein, O_CLOEXEC) == -1) {
        perror("pipein");
        exit(-1);
    }
    if (pipe2(pipeout, O_CLOEXEC) == -1) {
        perror("pipeout");
        exit(-1);
    }
    if (pipe2(pipeerr, O_CLOEXEC) == -1) {
        perror("pipeerr");
        exit(-1);
    }

    pid = fork();
    switch (pid) {
        case -1:
            perror("fork");
            return (-1);
        case 0:
            if (pipein[0] != STDIN_FILENO) {
                if (dup2(pipein[0], 0) == -1) {
                    perror("dup2");
                    exit(-1);
                }

                close(pipein[0]);
                close(pipein[1]);
            }

            if (pipeout[1] != STDOUT_FILENO) {
                if (dup2(pipeout[1], 1) == -1) {
                    perror("dup2");
                    exit(-1);
                }
                close(pipeout[0]);
                close(pipeout[1]);
            }

            if (pipeerr[1] != STDERR_FILENO) {
                if (dup2(pipeerr[1], 2) == -1) {
                    perror("dup2");
                    exit(-1);
                }
                close(pipeerr[0]);
                close(pipeerr[1]);
            }

            execlp(cmd, cmd, (char *) NULL);
            perror("execlp");
            _exit(EXIT_FAILURE);
    }

    if(timeout > 0) {
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = &alarm_signal;
        sigaction(SIGALRM, &sa, NULL);
        malarm(timeout);
    }

    close(pipein[0]);
    close(pipeout[1]);
    close(pipeerr[1]);
    *poutput = NULL;
    *perr = NULL;

    writepipe(pipein[1], input, length);
    close(pipein[1]);
    *poutput = readpipe(pipeout[0]);
    close(pipeout[0]);

    *perr = readpipe(pipeerr[0]);
    close(pipeerr[0]);

    retval = 0;
    wait(&status);

    if (WIFEXITED(status))
        retval = (WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        retval = (1024 + WTERMSIG(status));
    else if (WIFSTOPPED(status))
        retval = (1024 + WSTOPSIG(status));
    else if (WIFCONTINUED(status))
        retval = (1024 + SIGCONT);
    if (WCOREDUMP(status))
        retval = 1024 + WCOREDUMP(status);

    return retval;
}

int main(int argc, char** argv) {
    char *cmd, *inp, *out, *err;
    int return_value, tout;
    char tmp[132];

    if (argc == 4) {
        cmd = argv[1];
        inp = argv[2];
        tout = atoi(argv[3]);
    } else {
        return EXIT_FAILURE;
    }
    strcpy(tmp, inp);
    return_value = run(cmd, tmp, &out, &err, tout);

    free(out);
    free(err);
    return(return_value);
}
