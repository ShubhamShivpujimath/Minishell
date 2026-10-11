#include "header.h"
char prompt[20] = "minishell$";
char *external_commands[200];
int pid;
int status;
Slist *head = NULL;

int main()
{
    system("clear");


    char input_string[100];

    scan_input(prompt, input_string);


}