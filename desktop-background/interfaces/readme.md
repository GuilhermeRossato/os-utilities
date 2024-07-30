# Desktop Background Utility Interfaces

This directory contains scripts that expose the features from the parent utility program in other environments.

Each source file in this folder individually exports functions to perform each of the utility operations. They are isolated and use built-in language features to handle the utility executable as a sub-process and don't depend on external or third-party dependencies.

You **must** configure the path to the utility program executable for the script to execute is correctly. Each script has a `UTILITY_EXECUTABLE_PATH` variable at the start with a default value that must be correct, there is also a function to set the value during runtime.

## Examples

There is a usage example function on the begining of each source file. The call expression of the example function is commented out at the end of the file.

## Environments

 - [desktop-background.ts](./desktop-background.ts) - **Typescript** - Typed Modules - (imports / export)
 - [desktop-background.js](./desktop-background.js) - **Node.js** - CommonJS - (require / module.exports)
 - [desktop-background.mjs](./desktop-background.mjs) - **Node.js** - ES Modules - (import / export)
 - [desktop-background.py](./desktop-background.py) - **Python 3** - Asyncio
