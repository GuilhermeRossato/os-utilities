# Window State Utility Interfaces

This directory contains scripts that expose the features from the parent utility program in other environments.

Each source individually exports functions to perform each of the utility operations, they are isolated and use built-in modules to handle the utility executable as a sub-process and as such they don't depend on external or third-party dependencies.

You **must** configure the path to the utility program executable for the script to execute is correctly. Each script has a `UTILITY_EXECUTABLE_PATH` variable at the start with a default value that must be correct, there is also a function to set the value during runtime.

## Examples

There is a usage example function on the begining of each source file. The call expression of the example function is commented out at the end of the file.

## Environments

 - [window-state.ts](./window-state.ts) - **Typescript** - Typed Modules - (imports / export)
 - [window-state.js](./window-state.js) - **Node.js** - CommonJS - (require / module.exports)
 - [window-state.mjs](./window-state.mjs) - **Node.js** - ES Modules - (import / export)
 - [window-state.py](./window-state.py) - **Python 3** - Asyncio
