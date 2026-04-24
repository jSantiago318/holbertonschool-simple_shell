<<<<<<< HEAD
#ifndef SIMPLE_SHELL_H
#define SIMPLE_SHELL_H

#include <stdio.h>

extern char **environ;

int sh_run(char *prog_name);
char *sh_read_line(void);
int sh_execute(char *prog_name, char *line);

#endif /* SIMPLE_SHELL_H */
=======
#ifndef SIMPLE_SHELL_H
#define SIMPLE_SHELL_H

#include <stdio.h>

extern char **environ;

int sh_run(char *prog_name);
char *sh_read_line(void);
int sh_execute(char *prog_name, char *line);
char *sh_remap_command(char *line, char *remap);

#endif
>>>>>>> b612ae0171d41858be905d14c79bd0d2d6ef28ad
