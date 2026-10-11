#include "header.h"
extern int status;

void execute_internal_commands(char *input_string)
{
    //step 1 => compare and check input_string is exit or not
    if(strcmp(input_string, "exit") == 0)
    {
        exit(1);
    }

    //step 2 => compare and check input_string is pwd or not
    if(strcmp(input_string, "pwd") == 0)
    {
        //char *getcwd(char *buf, size_t size);
        char buf[256];
        getcwd(buf, 256);
        printf("%s\n", buf);
        status = 0;
    }

    //step 3 => compare and check input_string is cd or not
    if(strcmp(input_string, "cd") == 0 || strncmp(input_string, "cd ", 3) == 0)
    {
        // input_string => cd minishell
        int ret;

        if(strlen(input_string) <= 3)
        {
            //only cd => go to the home directory
            ret = chdir(getenv("HOME"));
        }
        else
        {
            ret = chdir(input_string + 3);
        }

        if(ret == 0)
        {
            status = 0;
        }
        else
        {
            perror("cd");
            status = 1 << 8;        /* exit status 1 */
        }
    }

    // step 4 => compare and check input_string is echo or not
    if(strncmp(input_string, "echo", 4) == 0 && (input_string[4] == ' ' || input_string[4] == '\0'))
    {
        // 4.1 => compare and check input_string is echo $? or not
        if(strcmp(input_string, "echo $?") == 0)
        {
            /*code for exit status of last command*/
            //use waitpid to get status of the last executed process and track it
            if(WIFEXITED(status))
            {
                printf("%d\n", WEXITSTATUS(status));
            }
            else if(WIFSIGNALED(status))
            {
                printf("%d\n", 128 + WTERMSIG(status));
            }
            else
            {
                printf("%d\n", 128 + WSTOPSIG(status));
            }
        }
        //step 4.2 => compare and check input_string is echo $SHELL or not
        else if(strcmp(input_string, "echo $SHELL") == 0)
        {
            char *shell = getenv("SHELL");
            printf("%s\n", shell ? shell : "minishell");
        }
        //step 4.3 => compare and check input_string is echo $$ or not
        else if(strcmp(input_string, "echo $$") == 0)
        {
            printf("%d\n", getpid());
        }
        else if(input_string[4] == '\0')
        {
            printf("\n");
        }
        else
        {
            //echo text
            printf("%s\n", input_string + 5);
        }
        status = 0;
    }
}

/* the child process executes the cmd using exec */
void execute_external_command(char *input_string)
{
    char *argv[20];
    int i = 0;

    //split the input_string into words
    char *token = strtok(input_string, " \t");
    while(token != NULL && i < 19)
    {
        argv[i++] = token;
        token = strtok(NULL, " \t");
    }
    argv[i] = NULL;

    if(argv[0] == NULL)
    {
        fprintf(stderr, "syntax error near unexpected token `|'\n");
        exit(2);
    }

    execvp(argv[0], argv);

    //exec failed
    perror(argv[0]);
    exit(127);
}

static int pipe_pids[10];
static int pipe_count = 0;

/* fg / bg send SIGCONT only to the process which runs the pipeline, it sends
 * the signal to the commands of the pipeline, they are stopped as well */
static void forward_sigcont(int signum)
{
    for(int i = 0; i < pipe_count; i++)
    {
        kill(pipe_pids[i], SIGCONT);
    }
}

/* the child process executes   cmd1 | cmd2 | ... | cmdN   */
void execute_n_pipe(char *input_string)
{
    char *cmds[10];
    int n = 0;
    int fd[2];
    int prev_fd = -1;
    int st = 0;

    //step 1 => split the input_string on '|'
    char *token = strtok(input_string, "|");
    while(token != NULL && n < 10)
    {
        cmds[n++] = token;
        token = strtok(NULL, "|");
    }

    signal(SIGCONT, forward_sigcont);

    //step 2 => create a pipe and a child process for every command
    for(int i = 0; i < n; i++)
    {
        if(i < n - 1)
        {
            pipe(fd);
        }

        int ret = fork();
        if(ret == 0)
        {
            signal(SIGCONT, SIG_DFL);

            if(prev_fd != -1)
            {
                //read from the previous command
                dup2(prev_fd, 0);
                close(prev_fd);
            }
            if(i < n - 1)
            {
                //write to the next command
                close(fd[0]);
                dup2(fd[1], 1);
                close(fd[1]);
            }
            execute_external_command(cmds[i]);
        }
        else if(ret < 0)
        {
            perror("fork");
            exit(1);
        }

        pipe_pids[i] = ret;
        pipe_count = i + 1;

        if(prev_fd != -1)
        {
            close(prev_fd);
        }
        if(i < n - 1)
        {
            close(fd[1]);
            prev_fd = fd[0];
        }
    }

    //step 3 => wait for all the commands, the status is the one of the last command
    for(int i = 0; i < n; i++)
    {
        waitpid(pipe_pids[i], &st, 0);
    }

    if(WIFEXITED(st))
    {
        exit(WEXITSTATUS(st));
    }
    exit(128 + WTERMSIG(st));
}