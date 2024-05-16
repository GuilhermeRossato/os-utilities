# window-state - Utility to interact with window states (Windows only)
 
A native program that reads and interact with the properties of window elements.

## Usage

```shell

  window-state [...filters] [..operations]

Filters: 

    --foreground         Select the focused window currently active (in focus).
    --handle <handle>    Select a window by its numeric handle id.
    --desktop            Select all children from the top-level desktop window object
    --parent <handle>    Select all children of a specific window.

Operations:

    --move <x> <y>       Move matches to a specific position.
    --size <w> <h>       Resize matches to a specific size.
    --show               Show matching windows (set visilibity to true).
    --hide               Hide matching windows (set visilibity to false).
    --maximize           Maximize matching windows.
    --minimize           Minimize matching windows.
    --set-foreground     Set the first match as the focused window.
    --set-top            Bring the first matching window to the top layer.
    --set-top-most       Bring the first matching window to the top-most layer.

Example: Move and resize the current foreground window
    window-state --foreground --move 10 10 --size 500 500
```

## State

The program will output the window states of all matching windows in a JSON list format if **when no operations are specified**. Each window state object follows this interface:

```ts
/**
 * Represents detailed information about a window.
 */
interface WindowState {
  /** The handle of the window. */
  handle: number;
  
  /** The title of the window. */
  title?: string;
  
  /** The top position of the window in pixels. */
  top?: number;
  
  /** The right position of the window. */
  right?: number;
  
  /** The bottom position of the window. */
  bottom?: number;
  
  /** The left position of the window. */
  left?: number;
  
  /** The module path associated with the window. */
  module?: string;
  
  /** The executable path related to the window. */
  executable?: string;
  
  /** The class name of the window. */
  classname?: string;
  
  /** The handle of the parent window or 0 if there is no parent. */
  parent: number;
  
  /** The handle of the next sibling window if one exists. */
  sibling?: number;
  
  /** The handle of the first child of the window if it has any. */
  child?: number;
  
  /** The process ID associated with the window. */
  pid: number;
  
  /** The thread ID related to the window. */
  thread: number;
  
  /** The numeric representation of the style attributes of the window (from GWL_STYLE). */
  style: number;
  
  /** The numeric representation of the extended style attributes of the window (from GWL_EXSTYLE). */
  exstyle: number;
  
  /** Indicates if the window is visible. */
  visible: boolean;
  
  /** Indicates if the window supports unicode. */
  unicode: boolean;
  
  /** True if the window is a popup. */
  popup?: boolean;
  
  /** True if the window has borders. */
  bordered?: boolean;
  
  /** True if the window is scrollable. */
  scrollable?: boolean;
  
  /** True if the window is contained within another window. */
  contained?: boolean;
  
  /** True if the window is minimized. */
  minimized?: boolean;
  
  /** True if the window is at the top layer. */
  topmost?: boolean;
  
  /** True if the window is transparent. */
  transparent?: boolean;
}
```

## Interface

The program can be executed as a stand-alone process by other programs.

The [interfaces](./interfaces/) folder contains scripts and usage examples in different environments.

## Compilation

The utility source is compiled into an executable with the following command:

```bash
cl.exe /nologo /Ob0 /O2 ./main.c /Fe"window-state.exe" user32.lib
```

The compilation steps for this program are stored at the [./compile.bat](./compile.bat) batch script. The script initializes the environment and loops between compiling and running it indefinitely (until the process is stopped by `Ctrl+C` or `Ctrl+D`).

The script that sets the compilation environment is located at `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat` and the compiler used is the accompanying `cl.exe` (Microsoft C/C++ Optimizing Compiler).