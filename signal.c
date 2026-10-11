#include "header.h"
extern char prompt[20];
extern int pid;

static int bg_pids[20];
static int bg_count = 0;

void signal_handler(int signum)
{
    //if child process is not present
    if(pid == 0)
    {
        printf("\n");
        printf("%s ", prompt);
        fflush(stdout);
    }
}

/* SIGCHLD => a background job terminated, clear the resources using waitpid() */
void sigchld_handler(int signum)
{
    int saved_errno = errno;
    int i = 0;

    while(i < bg_count)
    {
        if(waitpid(bg_pids[i], NULL, WNOHANG) > 0)
        {
            bg_pids[i] = bg_pids[--bg_count];
        }
        else
        {
            i++;
        }
    }
    errno = saved_errno;
}

void add_bg_pid(int p)
{
    sigset_t block, old;

    sigemptyset(&block);
    sigaddset(&block, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block, &old);

    if(bg_count < 20)
    {
        bg_pids[bg_count++] = p;
    }

    sigprocmask(SIG_SETMASK, &old, NULL);
}