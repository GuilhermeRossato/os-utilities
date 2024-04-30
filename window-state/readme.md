# Window State Utility
 
List, read, and write to the internal state of window elements

## Usage

```shell
Usage:
        window-state [...filters] [..actions]

Filtering:

                --desktop            Select the child window list from the root window list .
                --foreground         Filter windows by mafrom the active window (in focus).
                --handle <handle>    Select a target window by their handle number.
                --parent <handle>    Select the child window list from the child window list;
```

## Usage

The program can be used as a stand-alone console program.

## Compilation

The utility source is compiled into an executable with the following command:

```bash
cl.exe /nologo /Ob0 /O2 ./main.c /Fe"window-state.exe" user32.lib
```

The compilation is executed by [./compile.bat](./compile.bat), it continuously cycles between compilation and execution, to stop it you can press `Ctrl+C`.

The batch initializes the environment with a Microsoft Visual Studio 2019 script and compiles the program with the `cl.exe` C/C++ Optimizing Compiler.

