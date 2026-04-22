# simple_shell (Task 0.1 starter)

This project is a minimal UNIX command line interpreter for the first checker step.

## What this version supports

- Displays prompt `#holbieshell$ ` in interactive mode
- Reads one line with `getline`
- Handles EOF (`Ctrl+D`) by exiting cleanly
- Ignores blank/space-only lines
- Splits command by spaces/tabs
- Executes with `fork` + `execve`
- Waits for child with `waitpid`
- Prints errors with `perror`
- Passes `environ` to `execve`

## What this version does not support yet

- PATH search
- Builtins
- Advanced parsing (`;`, pipes, redirection, quotes)

## Compile

```sh
gcc -Wall -Werror -Wextra -pedantic -std=gnu89 *.c -o simple_shell
```

## Run

```sh
./simple_shell
```

Or non-interactive:

```sh
echo "/bin/ls" | ./simple_shell
```
