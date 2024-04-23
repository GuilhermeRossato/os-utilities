# Clipboard Text Utility (Windows)

Utility to read and write clipboard text data on Windows.

## Usage

```shell
  clipboard-text --read            / -r             Print the clipboard data stored in plain text format.
  clipboard-text --write <text...> / -w <text...>   Set the clipboard data from the text of the imediate program arguments.
  clipboard-text --file <path>     / -f <path>      Loads the text content of a file and write it to the clipboard data.
  clipboard-text --help            / -h             Print usage instructions and arguments.
```

## Usage

The program can be used as a stand-alone console program. The [interfaces](./interfaces/) folder contains scripts and usage examples to execute it in different environments.

## Compilation

The utility source is compiled into an executable with the following command:

```bash
cl.exe /nologo /Ob0 /O2 ./main.c /Fe"clipboard-text.exe" user32.lib
```

The compilation is executed with the [./compile.bat](./compile.bat) batch script which continuously cycles between compilation and execution. To stop it at any point you can press `Ctrl+C`.

The batch initializes the environment with a Microsoft Visual Studio 2019 script and compiles the program with the `cl.exe` C/C++ Optimizing Compiler.

The windows library `user32.lib` is linked on compilation as it provides the interfaces to read and write data to the clipboard.
