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
	(void)argc;
	return (sh_run(argv[0]));
}
