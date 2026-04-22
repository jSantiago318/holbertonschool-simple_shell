#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "simple_shell.h"

#define MAX_ARGS 64  /* Número máximo de argumentos de comando que podemos manejar */

/**
 * sh_read_line - Lee un comando del usuario
 *
 * Esta función espera a que el usuario escriba un comando y presione Enter.
 * Lee toda la línea (incluyendo espacios) y elimina el carácter de nueva línea.
 *
 * Return: La línea de comando como una cadena, o NULL si el usuario presionó Ctrl+D o hay error
 */
char *sh_read_line(void)
{
	char *line;
	size_t cap;
	ssize_t nread;

	line = NULL;      /* getline asignará memoria para nosotros */
	cap = 0;          /* getline determinará cuánto espacio necesitamos */
	nread = getline(&line, &cap, stdin);
	
	/* getline retorna -1 si el usuario presionó Ctrl+D (EOF) o si hubo un error */
	if (nread == -1)
	{
		free(line);     /* Liberar memoria antes de retornar */
		return (NULL);
	}

	/* getline incluye el carácter de nueva línea al final.
	   Lo removemos porque hace más fácil analizar el comando después. */
	if (nread > 0 && line[nread - 1] == '\n')
		line[nread - 1] = '\0';  /* Reemplazar salto de línea con marcador de fin de cadena */

	return (line);
}

/**
 * sh_execute - Ejecuta un comando creando un nuevo proceso
 *
 * Esta función divide la entrada del usuario en partes (como "ls -l /tmp" se convierte
 * en ["ls", "-l", "/tmp"]), luego crea un proceso hijo y ejecuta el comando en él.
 *
 * @prog_name: El nombre del programa (usado para mensajes de error)
 * @line: El comando del usuario como una cadena
 *
 * Return: El estado de salida del comando, o 1 si algo salió mal
 */
int sh_execute(char *prog_name, char *line)
{
	pid_t pid;          /* ID del proceso para el proceso hijo que crearemos */
	int wstatus;        /* Almacenará el estado de salida del hijo */
	char *argv[MAX_ARGS];  /* Array para almacenar el comando y sus argumentos */
	char *cmd;          /* El comando en sí (primera palabra que escribió el usuario) */
	char *token;        /* Variable temporal para dividir la entrada */
	int i;              /* Contador para construir el array argv */

	/* PASO 1: Dividir la entrada en palabras (separadas por espacios o tabulaciones)
	   Ejemplo: "ls -l /tmp" se convierte en ["ls", "-l", "/tmp", NULL]
	   
	   strtok() divide la cadena en pedazos. Cada pedazo se detiene en un espacio o tabulación.
	   La primera llamada usa la línea de entrada. Las llamadas posteriores usan NULL para continuar. */
	
	cmd = strtok(line, " \t");  /* Obtener la primera palabra (nombre del comando) */
	if (cmd == NULL)  /* El usuario solo presionó Enter sin comando */
		return (0);   /* Nada que hacer, solo retornar éxito */

	argv[0] = cmd;    /* El primer elemento siempre es el nombre del comando */
	i = 1;
	
	/* Obtener las palabras restantes y guardarlas en argv */
	token = strtok(NULL, " \t");  /* Obtener siguiente palabra */
	while (token != NULL && i < MAX_ARGS - 1)  /* Continuar hasta que no haya más palabras */
	{
		argv[i] = token;  /* Guardar este argumento */
		i++;
		token = strtok(NULL, " \t");  /* Obtener siguiente palabra */
	}
	argv[i] = NULL;  /* El último elemento debe ser NULL para marcar el final */

	/* PASO 2: Crear un nuevo proceso para ejecutar el comando
	   fork() crea una copia de nuestro programa shell. Ahora tenemos DOS procesos:
	   - El padre (shell original) esperará a que el comando termine
	   - El hijo ejecutará el comando y saldrá */
	
	pid = fork();
	if (pid == -1)  /* fork() falló */
	{
		perror(prog_name);  /* Imprimir mensaje de error */
		return (1);         /* Decirle al shell que algo salió mal */
	}

	if (pid == 0)  /* Estamos en el proceso HIJO */
	{
		/* PASO 3: Reemplazar este proceso hijo con el comando actual
		   execve() ejecuta el comando en este proceso, reemplazándolo completamente.
		   El comando obtiene nuestro argv (los argumentos) y environ (variables de entorno).
		   
		   Si execve() funciona, nunca retorna - el proceso hijo es ahora el comando.
		   Si falla, execve() retorna e imprimimos un error. */
		
		execve(cmd, argv, environ);

		/* Si llegamos aquí, execve falló (comando no encontrado, etc.) */
		perror(prog_name);
		_exit(127);  /* Salir del hijo con código de error 127 */
	}

	/* Estamos de vuelta en el proceso PADRE (aún el shell) */
	/* PASO 4: Esperar a que el proceso hijo termine
	   Sin waitpid(), el hijo se convierte en un "zombie" - aún usando recursos
	   pero no ejecutándose. Debemos esperar a que se limpie adecuadamente. */
	
	if (waitpid(pid, &wstatus, 0) == -1)
	{
		perror(prog_name);
		return (1);
	}

	/* Verificar si el hijo salió normalmente (no de una señal)
	   y obtener su código de salida para retornar */
	if (WIFEXITED(wstatus))
		return (WEXITSTATUS(wstatus));  /* Retornar el estado de salida del comando */

	return (1);
}

/**
 * sh_run - El ciclo principal del shell
 *
 * Este es el corazón del shell. Se ejecuta indefinidamente, pidiendo al usuario
 * comandos y ejecutándolos. Solo se detiene cuando el usuario presiona Ctrl+D.
 *
 * @prog_name: El nombre del programa (usado para mensajes de error)
 *
 * Return: El estado de salida del último comando ejecutado
 */
int sh_run(char *prog_name)
{
	char *line;
	int interactive;  /* ¿Está el usuario escribiendo comandos, o estamos leyendo de una tubería? */
	int status;       /* Estado del último comando */

	/* isatty() verifica si stdin está conectado a una terminal (usuario escribiendo)
	   o si es una tubería/archivo (entrada automatizada).
	   interactive = 1 si una persona está escribiendo, 0 si la entrada está tuberizada. */
	interactive = isatty(STDIN_FILENO);
	status = 0;  /* Ningún comando ejecutado aún, así que empezar con éxito */

	/* Ciclo infinito: seguir ejecutándose hasta que el usuario salga */
	while (1)
	{
		/* Solo mostrar el símbolo del sistema si una persona real está escribiendo
		   (no mostrarlo cuando los comandos se tuberizar) */
		if (interactive)
			write(STDOUT_FILENO, "HolbieShell$ ", 13);  /* Imprimir el símbolo del sistema */

		/* Pedir un comando al usuario */
		line = sh_read_line();
		
		/* Si el usuario presionó Ctrl+D, line será NULL */
		if (line == NULL)
		{
			/* Imprimir una nueva línea para que la salida se vea bien */
			if (interactive)
				write(STDOUT_FILENO, "\n", 1);
			break;  /* Salir del ciclo (y terminar el shell) */
		}

		/* Ejecutar el comando y recordar su estado de salida */
		status = sh_execute(prog_name, line);
		free(line);  /* ¡No olvides liberar la memoria que asignamos! */
	}

	return (status);
}
