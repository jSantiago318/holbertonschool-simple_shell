#include "simple_shell.h"

/**
 * main - entry point for the simple shell template
 * @argc: argument count
 * @argv: argument vector
 *
 * Return: shell exit status
 */
int main(int argc, char **argv)
{
	(void)argc; /* Silienciamos el warning: argc no se usa directamente */
	return (sh_run(argv[0]));
}
