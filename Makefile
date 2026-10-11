CC = gcc
CFLAGS = -Wall
SRC = main.c scan.c signal.c check_cmd.c execute_cmds.c list.c

minishell: $(SRC) header.h
	$(CC) $(CFLAGS) $(SRC) -o minishell

clean:
	rm -f minishell