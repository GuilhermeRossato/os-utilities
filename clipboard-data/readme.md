# Clipboard-get

Native Windows utility to read data from the clipboard.

## Usage

```
    clipboard-get 1         Retrieves the clipboard data stored in the plain text format.
    clipboard-get <format>  Retrieves the clipboard data stored in the specified format.
    clipboard-get --list    List each clipboard format that contains data to be read.
    clipboard-get --json    Prints the list to the output in a parseable JSON format.
    clipboard-get --help    Display usage instructions and arguments.
```

Note: When displaying the format list in text mode, the lack of available data will be indicated by a format of code 0.
Obs:  If there is no data stored in the specified clipboard format the program will exit exit with a non-zero code.

## Compilation

The compilation script for this program is described at the [./compile.bat](./compile-loop.bat) batch script.

It attempts compilation and execution and then repeats the process on key press unless `Ctrl+C` is pressed (to stop).

The batch script internally uses the `cl.exe` C/C++ Optimizing Compiler from Microsoft Visual Studio 2019 to compile the program which imports the main `windows.h` header and is linked with `user32.lib`.

The script that sets the compilation environment is located at `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat`.