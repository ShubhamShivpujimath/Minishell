#include "header.h"
extern char *external_commands[200];
extern int pid;
extern int status;
extern Slist *head;

void scan_input(char *prompt, char *input_string)
{
    //Requirements
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, sigchld_handler);

    //1. Prompt
    while (1)
    {
        printf("%s ", prompt);
        fflush(stdout);
        if(scanf(" %99[^\n]", input_string) == EOF)
        {
            // Ctrl + D
            printf("exit\n");
            exit(0);
        }

        /* validate the input string is PS1 or not*/
        if(strncmp(input_string, "PS1=", 4) == 0)
        {
            // My input string is PS1
            // step1 => validate space is present or not
            if(input_string[4] == ' ')
            {
                //printf -> command not found
                printf("%s: command not found\n", input_string);
            }
            else 
            {
                strncpy(prompt, input_string+4, 19);
                prompt[19] = '\0';
            }

        }
        else
        {
            // either external or internal or special variable
            char *cmd = get_command(input_string);

            // extract the external commands
            extract_external_commands(external_commands);

            int ret = check_command_type(cmd);

            if(ret == BUILTIN)
            {
                execute_internal_commands(input_string);
            }
            else if(ret == EXTERNAL)
            {
                //create a child process
                pid = fork();
                if(pid == 0)
                {
                    //signals must work in the default way for the child
                    signal(SIGINT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    signal(SIGCHLD, SIG_DFL);

                    //check pipe is present or not
                    if(strchr(input_string, '|') != NULL)
                    {
                        // if present, call execute_n_pipe()
                        execute_n_pipe(input_string);
                    }
                    else
                    {
                        //else pipe not present, execute the cmd using exec
                        execute_external_command(input_string);
                    }
                }
                else if(pid > 0)
                {
                    waitpid(pid, &status, WUNTRACED);

                    if(WIFSTOPPED(status))
                    {
                        //store the stopped process info into the linked list
                        insert_at_first(&head, pid, input_string);
                        printf("\n[%d]+  Stopped\t%s\n", head->id, head->cmd);
                    }
                    else if(WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
                    {
                        printf("\n");
                    }

                    //child process is not present any more
                    pid = 0;
                }
                else
                {
                    perror("fork");
                    pid = 0;
                }
            }
            else if(strcmp(input_string, "jobs") == 0)
            {
                //print the linked list
                print_list(head);
            }
            else if(strcmp(input_string, "fg") == 0)
            {
                //Implement fg => continue the stop process in foreground
                if(head == NULL)
                {
                    printf("fg: no current job\n");
                }
                else
                {
                    char job_cmd[100];

                    pid = head->pid;
                    strcpy(job_cmd, head->cmd);
                    printf("%s\n", job_cmd);

                    kill(pid, SIGCONT);
                    delete_At_first(&head);
                    waitpid(pid, &status, WUNTRACED);

                    if(WIFSTOPPED(status))
                    {
                        insert_at_first(&head, pid, job_cmd);
                        printf("\n[%d]+  Stopped\t%s\n", head->id, head->cmd);
                    }
                    else if(WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
                    {
                        printf("\n");
                    }
                    pid = 0;
                }
            }
            else if(strcmp(input_string, "bg") == 0)
            {
                //Implement bg => continue the stop process in background
                if(head == NULL)
                {
                    printf("bg: no current job\n");
                }
                else
                {
                    printf("[%d]+ %s &\n", head->id, head->cmd);

                    //if(SIGCHILD) => is to identify child get terminated, clear the resources using waitpid()
                    add_bg_pid(head->pid);
                    kill(head->pid, SIGCONT);
                    delete_At_first(&head);
                }
            }
            else
            {
                //No command
                printf("%s: command not found\n", cmd);
                status = 127 << 8;      /* exit status 127 */
            }
        }
    }
}

char *get_command(char *input_string)
{
    // return the first word of the input string
    static char command[100];
    int i = 0;

    while(input_string[i] != '\0' && input_string[i] != ' ' && input_string[i] != '|' && i < 99)
    {
        command[i] = input_string[i];
        i++;
    }
    command[i] = '\0';

    return command;
}