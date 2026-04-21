#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "simple_shell.h"

#define MAX_ARGS 64

/**
 * sh_read_line - read one line from stdin
 *
 * Return: heap-allocated line without trailing newline, or NULL on EOF/error
 */
char *sh_read_line(void)
{
	char *line;
	size_t cap;
	ssize_t nread;

	line = NULL;
	cap = 0;
	nread = getline(&line, &cap, stdin);
	if (nread == -1)
	{
		/* EOF (Ctrl+D) or read error */
		free(line);
		return (NULL);
	}

	/* Remove the trailing newline so tokenization is easier later. */
	if (nread > 0 && line[nread - 1] == '\n')
		line[nread - 1] = '\0';

	return (line);
}

/**
 * sh_execute - execute one command line using fork + execve
 * @prog_name: argv[0] from main (used by perror)
 * @line: input line buffer
 *
 * Return: child status on success path, 1 on shell-side failure
 */
int sh_execute(char *prog_name, char *line)
{
	pid_t pid;
	int wstatus;
	char *argv[MAX_ARGS];
	char *cmd;
	char *token;
	int i;

	/*
	 * Split by spaces/tabs.
	 * This keeps the parser simple while supporting cases like:
	 * "./hbtn_ls /var"
	 */
	cmd = strtok(line, " \t");
	if (cmd == NULL)
		return (0);

	argv[0] = cmd;
	i = 1;
	token = strtok(NULL, " \t");
	while (token != NULL && i < MAX_ARGS - 1)
	{
		argv[i] = token;
		i++;
		token = strtok(NULL, " \t");
	}
	argv[i] = NULL;

	/* Create child process for command execution. */
	pid = fork();
	if (pid == -1)
	{
		perror(prog_name);
		return (1);
	}

	if (pid == 0)
	{
		/* Child replaces its image with the target program. */
		execve(cmd, argv, environ);

		/* execve only returns when there is an error. */
		perror(prog_name);
		_exit(127);
	}

	/* Parent waits for child so we do not leave zombie processes. */
	if (waitpid(pid, &wstatus, 0) == -1)
	{
		perror(prog_name);
		return (1);
	}

	if (WIFEXITED(wstatus))
		return (WEXITSTATUS(wstatus));

	return (1);
}

/**
 * sh_run - interactive/non-interactive shell loop
 * @prog_name: argv[0] from main
 *
 * Return: last command status
 */
int sh_run(char *prog_name)
{
	char *line;
	int interactive;
	int status;

	interactive = isatty(STDIN_FILENO);
	status = 0;

	while (1)
	{
		/* Print prompt only in interactive mode (not when piped). */
		if (interactive)
			write(STDOUT_FILENO, "#cisfun$ ", 9);

		line = sh_read_line();
		if (line == NULL)
		{
			if (interactive)
				write(STDOUT_FILENO, "\n", 1);
			break;
		}

		status = sh_execute(prog_name, line);
		free(line);
	}

	return (status);
}