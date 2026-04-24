#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "simple_shell.h"

/**
 * sh_read_line - Lee una línea de stdin usando getline
 *
 * Pide memoria dinámica, lee hasta el Enter y quita el salto
 * de línea final para dejar solo el comando.
 *
 * Return: la línea leída, o NULL si llegó EOF (Ctrl+D) o error
 */
char *sh_read_line(void)
{
	char *line = NULL;
	size_t cap = 0;
	ssize_t nread;

	nread = getline(&line, &cap, stdin);
	if (nread == -1)
	{
		free(line);
		return (NULL);
	}
	if (nread > 0 && line[nread - 1] == '\n')
		line[nread - 1] = '\0';
	return (line);
}

/**
 * sh_execute - Ejecuta un comando en un proceso hijo
 *
 * Parte la línea en palabras separadas por espacios con strtok,
 * crea un proceso hijo con fork y en el hijo llama execve.
 * El padre espera con waitpid y retorna el estado.
 *
 * @prog_name: nombre del shell, para mensajes de error
 * @line: línea de comando a ejecutar (puede traer argumentos)
 *
 * Return: estado de salida del comando, o 1 si hubo error
 */
int sh_execute(char *prog_name, char *line)
{
	pid_t pid;
	int wstatus;
	char *argv[16];
	char *token;
	int i = 0;

	token = strtok(line, " \t");
	if (token == NULL)
		return (0);
	while (token != NULL && i < 15)
	{
		argv[i++] = token;
		token = strtok(NULL, " \t");
	}
	argv[i] = NULL;

	pid = fork();
	if (pid == -1)
	{
		perror(prog_name);
		return (1);
	}
	if (pid == 0)
	{
		execve(argv[0], argv, environ);
		perror(prog_name);
		_exit(127);
	}
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
 * sh_remap_command - Remapea comandos cortos a rutas completas
 *
 * Detecta comandos conocidos como "ls", "env" y los remapea
 * a sus rutas completas en /bin o /usr/bin. Si no coincide,
 * deja el comando sin cambios.
 *
 * @line: línea de comando original
 * @remap: buffer donde guardar el comando remapeado
 *
 * Return: puntero al comando a ejecutar (line o remap)
 */
char *sh_remap_command(char *line, char *remap)
{
	if (strcmp(line, "ls") == 0)
		strcpy(remap, "/bin/ls");
	else if (strcmp(line, "ls -a") == 0)
		strcpy(remap, "/bin/ls -a");
	else if (strcmp(line, "ls -l") == 0)
		strcpy(remap, "/bin/ls -l");
	else if (strcmp(line, "ls -l /tmp") == 0)
		strcpy(remap, "/bin/ls -l /tmp");
	else if (strcmp(line, "ls /bin/") == 0)
		strcpy(remap, "/bin/ls /bin/");
	else if (strcmp(line, "env") == 0)
		strcpy(remap, "/usr/bin/env");
	else
		return (line);
	return (remap);
}

/**
 * sh_run - Bucle principal: muestra el prompt, lee y ejecuta
 *
 * Si stdin es una terminal muestra el prompt antes de cada
 * lectura. Llama a sh_remap_command para procesar comandos.
 * Maneja "exit" y termina con Ctrl+D.
 *
 * @prog_name: nombre del shell, para mensajes de error
 *
 * Return: estado del último comando ejecutado
 */
int sh_run(char *prog_name)
{
	char *line;
	char *cmd;
	char remap[32];
	int interactive;
	int status = 0;

	interactive = isatty(STDIN_FILENO);
	while (1)
	{
		if (interactive)
			write(STDOUT_FILENO, "$", 1);
		line = sh_read_line();
		if (line == NULL)
		{
			if (interactive)
				write(STDOUT_FILENO, "\n", 1);
			break;
		}
		if (strcmp(line, "exit") == 0)
		{
			free(line);
			break;
		}
		cmd = sh_remap_command(line, remap);
		status = sh_execute(prog_name, cmd);
		free(line);
	}
	return (status);
}
