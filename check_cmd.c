#include "header.h"
extern char *external_commands[200];

int check_command_type(char *command)
{
    // Check whether the cmd external or internal or special variable
    /* List of builtin commands */
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};

    
    // step 1=> check whether the command is BUILTIN or not
    for(int i = 0; builtins[i] != NULL; i++)
    {
        //use strcmp and compare builtins[i] with command
        // if equal => return BUILTIN
        if(strcmp(builtins[i], command) == 0)
        {
            return BUILTIN;
        }
    }

    //step 2=> Check the command is external or not
    for(int i = 0; external_commands[i] != NULL; i++)
    {
        // use strcmp and compare external_commands[i] with command
        // if equal=> return EXTERNAL
        if(strcmp(external_commands[i], command) == 0)
        {
            return EXTERNAL;
        }
    }

    return NO_COMMAND;
}

void extract_external_commands(char **external_commands)
{
    //the commands are already extracted
    if(external_commands[0] != NULL)
    {
        return;
    }

    char buffer[100];
    int i = 0;

    //step 1 => open the file in the read mode
    FILE *fp = fopen("external_cmds.txt", "r");
    if(fp == NULL)
    {
        printf("external_cmds.txt: file not found\n");
        return;
    }

    //step 2 => read the commands one by one
    while(fgets(buffer, sizeof(buffer), fp) != NULL && i < 199)
    {
        //remove the new line
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if(buffer[0] == '\0')
        {
            continue;
        }

        //step 3 => allocate the memory dynamically
        external_commands[i] = malloc(strlen(buffer) + 1);

        //step 4 => store the command in the 2d array
        strcpy(external_commands[i], buffer);
        i++;
    }
    external_commands[i] = NULL;

    fclose(fp);
}