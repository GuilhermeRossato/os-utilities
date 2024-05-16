# clipboard-text - Utility to interact with the clipboard data stored in text format (Window only)

A native program that reads and writes data from the clipboard in text format.

## Usage

```shell

    clipboard-text <mode> [options...]

Modes: 

    --read                 Print the current clipboard plain text data.
    --write <text...>      Update the clipboard data from the text of the program arguments.
    --file <path>          Write the clipboard text from the contents of the specified text file.

Example: Replace the clipboard with \"Hello word\"
    clipboard-text --write Hello world
```

Argument has an alias of its first character (`--read` can be reduced to `-r`, etc).

## Interface

The program can be executed as a stand-alone process by other programs.

The [interfaces](./interfaces/) folder contains scripts and usage examples in different environments.
## Compilation

The source is compiled into an executable with the following command:

```bash
cl.exe /nologo /Ob0 /O2 ./main.c /Fe"clipboard-text.exe" user32.lib
```

The compilation steps for this program are stored at the [./compile.bat](./compile.bat) batch script. The script initializes the environment and loops between compiling and running it indefinitely (until the process is stopped by `Ctrl+C` or `Ctrl+D`).

The windows library `user32.lib` is linked on compilation as it provides the interfaces to read and write data to the clipboard.