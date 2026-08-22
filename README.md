# ash

A Unix shell written from scratch in C 

## Features

- Command execution via `fork`/`execvp`, with `$PATH` lookup
- Pipelines: `cmd1 | cmd2 | cmd3`
- Logical chaining: `&&`, `||`, `;`
- Background jobs: `cmd &`
- Subshells `( ... )` and grouped commands `{ ... }`
- Command substitution: `` $(cmd) ``
- Variable expansion: environment variables, `$?` (last exit status), `$$` (PID)
- Quoting (`"..."`, `'...'`) with fragment concatenation, e.g. `foo"$X"bar`
- Builtins: `cd`, `exit`

## Not yet supported

See [TODO.md](TODO.md) for the full list. Notably missing right now:

- I/O redirection (`>`, `<`, `>>`)
- Job control (no process groups / signal handling for background jobs)
- Shell-local variables (only environment variables are expanded)
- Line editing (no history, arrow keys, or tab completion)
- An `rc`/config file

## Build & run

Requires `gcc` and `make`. No other dependencies.

```
make build   # compile to bin/ash
make run     # run bin/ash (does not rebuild)
make all     # build then run (default target)
make clean   # remove bin/ and obj/
```

## Architecture

```
input   -> reads a line of input
parser  -> recursive parse into a SyntaxTreeNode tree
execute -> walks the tree, forking/executing as needed
```

All per-line memory is allocated from a bump arena (`src/utils/arena`) that's
created fresh for each input line and torn down after it runs — parser and
execute code never free individual nodes or strings. `String`
(`src/utils/str`) is a length-prefixed buffer used everywhere in place of raw
`char *`; conversion to a null-terminated C string only happens at the edge,
right before an `exec`-family call.

Grammar, roughly:

```
CMD_LIST  -> AND_OR (';' | '&') ...
AND_OR    -> PIPE ('&&' | '||') ...
PIPE      -> (CMD | '(' CMD_LIST ')' | '{' CMD_LIST '}') ('|' ...)
CMD       -> ARG ARG ...
ARG       -> (TEXT | VAR | CMD_SUBST)*
```

There's no test suite yet 
