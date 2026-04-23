# simple_shell

This project is a minimal UNIX command line interpreter for the first checker step.

## What this version supports

- Displays prompt `holbieshell$ ` in interactive mode
- Reads one line with `getline`
- Handles EOF (`Ctrl+D`) 
- Handles 'ls' , 'ls -a' ,...
 
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
