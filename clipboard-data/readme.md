# clipboard-data - Utility to access clipboard data (Window only)

A native program to read and write data from the clipboard on Windows.

## Usage

```shell

    clipboard-data <mode> [options...]
  
Modes:

  --read / --get         Output the current plain text data of the clipboard.
  --read <format>        Output the clipboard data stored in a specified format.
  --write <text...>      Set the clipboard text from the imediate program arguments.
  --file <file-path>     Set the clipboard text from the contents of a file.
  --list                 List all the clipboard formats available in a json array string.
  
Example: Replace the clipboard with \"Hello word\"
    clipboard-data --write Hello world
```

Each option argument has an alias of its first character (`--read` can be reduced to `-r`, `--list` to `-l`, etc).

## Interface

The program can be executed as a stand-alone process by other programs.

The [interfaces](./interfaces/) folder contains scripts and usage examples in different environments.

## Compilation

The compilation steps for this program are stored at the [./compile.bat](./compile.bat) batch script.

The batch script initializes the environment and loops between compiling and running it indefinitely (until the process is stopped by `Ctrl+C` or `Ctrl+D`).

The script that sets the compilation environment is located at `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat` and the compiler used is the accompanying `cl.exe` (Microsoft C/C++ Optimizing Compiler).