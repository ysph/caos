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

#define NN 132

int start(const char* cmd, int *fd);

int getnum(FILE *f, int *px)
{
    if (fscanf(f, "%d", px) == 1)
        return -1;
    return 0;
}

int main(int argc, char* argv[])
{
    char *p1, *p2;
    pid_t pid[2];
    int p1fd[3], p2fd[3];
    FILE *p1stdin, *p1stdout;
    FILE *p2stdin, *p2stdout;
#if STDERR
#else
    FILE *p1stderr, *p2stderr;
#endif
    int status;
    int x,y;
    int k;
#if LINE
    char lin[NN];
#endif
    if (argc == 3)
    {
        p1 = argv[1];
        p2 = argv[2];
        if (p1 > (1 << 12) && p2 > (2 << 12)) return 1;
    } else {
        return 1;
    }

    pid[0] = start(p1, p1fd);
    pid[1] = start(p2, p2fd);
    p1stdin = fdopen(p1fd[0], "w");
    p1stdout = fdopen(p1fd[1], "r");
    p2stdin = fdopen(p2fd[0], "w");
    p2stdout = fdopen(p2fd[1], "r");
#if STDERR
#else
    p1stderr = fdopen(p1fd[2], "r");
    p2stderr = fdopen(p2fd[2], "r");
#endif
#if DEBUG
    sleep(1);
#endif
#if LINE
    while(!feof(stdin))
    {
        if (fgets(lin, NN, stdin))
        {
            fputs(lin, p1stdin);
            fflush(p1stdin);
            if (fgets(lin, NN, p1stdout))
            {
                fputs(lin, p2stdin);
                fflush(p2stdin);
                if (fgets(lin, NN, p2stdout))
                {
                    fputs(lin,stdout);
                }
            }
        }
    }
#else
    while(!feof(stdin))
    {
        if (!getnum(stdin, &x))
            break;
#if DEBUG
        fprintf(stderr, "x(stdin)=%d\n", x);
#endif
        fprintf(p1stdin, "%d\n", x);
        fflush(p1stdin);
        getnum(p1stdout, &y);
        if (!(y&1))
        {
            x = y;
            fprintf(p2stdin, "%d\n", x);
            fflush(p2stdin);
            getnum(p2stdout, &y);
        }
        printf("%d\n", y);
    }
    fclose(p1stdin);
    close(p1fd[0]);
    fclose(p2stdin);
    close(p2fd[0]);
#endif
    fclose(p1stdout);
    fclose(p2stdout);
#if STDERR
#else
    fclose(p1stderr);
    fclose(p2stderr);
#endif
    for(k = 0; k < 3; k++)
    {
        close(p1fd[k]);
        close(p2fd[k]);
    }

    for(k = 0; k < 2; k++)
    {
        waitpid(pid[k], &status, 0);
    }

    return 0;
}

//-------------------------------------
int start (const char* cmd, int *fd)
{
    int pipein[2], pipeout[2], pipeerr[2];
    pid_t pid;
    int dp2;

    if (pipe2(pipein, O_CLOEXEC) == -1)
    {
        perror("pipein");
        exit(-1);
    }
    if (pipe2(pipeout, O_CLOEXEC) == -1)
    {
        perror("pipeout");
        exit(-1);
    }
    if (pipe2(pipeerr, O_CLOEXEC) == -1)
    {
        perror("pipeerr");
        exit(-1);
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(-1);
    }

    if (!pid)
    {
        dp2 = dup2(pipein[0], 0);
#if STDERR
        if ( dp2 == -1)
        {
            perror("dp2(0)");
            exit(-1);
        }
#endif
        close(pipein[0]);
        close(pipein[1]);

        dp2 = dup2(pipeout[1], 1);
#if STDERR
        if ( dp2 == -1)
        {
            perror("dp2(1)");
            exit(-1);
        }
#endif
        close(pipeout[0]);
        close(pipeout[1]);
#if STDERR
#else
        dup2(pipeerr[1], 2);
        close(pipeerr[0]);
        close(pipeerr[1]);
#endif
#if DEBUG
        fprintf(stderr, "execlp\n");
#endif
        dp2 = dp2;
        execlp(cmd, cmd, NULL);
        perror("execlp");
        _exit(1);
    }

    close(pipein[0]);
    close(pipeout[1]);
    close(pipeerr[1]);
    fd[0] = pipein[1];
    fd[1] = pipeout[0];
#if STDERR
    close(pipeerr[0]);
#else
    fd[2] = pipeerr[0];
#endif

    return pid;
}
