# desktop-background - Utility to interact with the desktop background image (Windows only)
 
A native program to retrieve and replace the background image on the desktop.

## Usage

```shell

  desktop-background --help / -h / -v     Display the help text
  desktop-background --get / -g / --read  Retrieve the current desktop background image in JSON object format
  desktop-background --set <file>         Update the current desktop image to the specified image by path
```

## Output 

The program outputs a JSON object string (that always starts with `{` and ends with `}`) with either a "path" property or an "error" property.

When successfull the object contains a `path` property with the file path of the current desktop background image as stored at the Window Registry. When the program fails an `error` property will be present with the error details.

When setting the background image to a target file path specified as a program argument the registry is updated and the same target is printed in the `path` property unless an error occurs. The `.bmp` image format is widely supported and the program will fail silently when the target file does not exist or its file format is not supported.

## Interface

The program can be executed as a stand-alone process by other programs by providing the correct arguments. The [interfaces](./interfaces/) folder contains scripts and usage examples for different environments.

## Compilation

The utility source is compiled into an executable with the following command:

```bash
cl.exe /nologo /Ob0 /O2 ./main.c /Fe"desktop-background.exe" advapi32.lib user32.lib
```

The compilation steps for this program are stored at the [./compile.bat](./compile.bat) batch script. The script initializes the environment and loops between compiling and running it indefinitely (until the process is stopped by `Ctrl+C` or `Ctrl+D`).

The script that sets the compilation environment is located at `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat` and the compiler used is the accompanying `cl.exe` (Microsoft C/C++ Optimizing Compiler).