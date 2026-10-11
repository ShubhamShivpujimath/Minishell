#include "header.h"

static int next_id = 1;

void insert_at_first(Slist **head, int pid, char *cmd)
{
    Slist *new = malloc(sizeof(Slist));
    if(new == NULL)
    {
        return;
    }

    if(*head == NULL)
    {
        next_id = 1;
    }

    new->id = next_id++;
    new->pid = pid;
    strncpy(new->cmd, cmd, sizeof(new->cmd) - 1);
    new->cmd[sizeof(new->cmd) - 1] = '\0';
    new->link = *head;
    *head = new;
}

void delete_At_first(Slist **head)
{
    if(*head == NULL)
    {
        return;
    }

    Slist *temp = *head;
    *head = temp->link;
    free(temp);
}

void print_list(Slist *head)
{
    while(head)
    {
        printf("[%d]  Stopped\t%s\n", head->id, head->cmd);
        head = head->link;
    }
}