#ifndef SIMPLE_SHELL_H
#define SIMPLE_SHELL_H

#include <stdio.h>

extern char **environ;

int sh_run(char *prog_name);
char *sh_read_line(void);
int sh_execute(char *prog_name, char *line);
char *sh_remap_command(char *line, char *remap);

#endif
