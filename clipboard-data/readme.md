# clipboard-data - Utility to access clipboard data (on Window)

A native program used to read data from the clipboard in any format for Windows.

## Usage

```shell
  clipboard-data <format>      Retrieves the clipboard data stored in the specified format.
  clipboard-data --list        List each clipboard format that contains data to be read.
  clipboard-data --json        Prints the list of formats in a parseable JSON format.
  clipboard-data --help        Display usage instructions and arguments.
  
  Ex: clipboard-data 1    Retrieves the text format stored in the clipboard
```

Each option argument has an alias of its first character (`--list` can be reduced to `-l`, etc).

Note:   When displaying the format list in text mode, the lack of available data will be indicated by a format of code 0.
Note 2: If there is no data stored in the specified clipboard format the program will exit exit with a non-zero code.

## Interface

The program can be used as a stand-alone process and the [interfaces](./interfaces/) folder contains scripts and usage examples in different environments.
## Compilation

The compilation steps for this program are stored at the [./compile.bat](./compile.bat) batch script.

The batch script initializes the environment and loops between compiling and running it indefinitely (until the process is stopped by `Ctrl+C` or `Ctrl+D`).

The script that sets the compilation environment is located at `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat` and the compiler used is the accompanying `cl.exe` (Microsoft C/C++ Optimizing Compiler).