# Clipboard Text Utility Interfaces (Windows)

This directory contains scripts that expose the features from the parent utility program in other environments.

Each source individually exports functions to perform each of the utility operations, they are isolated and use built-in modules to handle the utility executable as a sub-process and as such they don't depend on external or third-party dependencies.

You **must** configure the path to the utility program executable for the script to execute is correctly. Each script has a `UTILITY_EXECUTABLE_PATH` variable at the begining with a default value that you can set manually, but you can also call the update utility executable path function to set this value before using the other exported functions.

## Usage

There is a usage example function on each source file. The call expression of the example function is commented out at the end of the file.

## Environments

 - [clipboard-text.js](./clipboard-text.js) - Node.js - CommonJS - (require / module.exports)
 - [clipboard-text.mjs](./clipboard-text.mjs) - Node.js - ES Module - (import ... from)
 - [clipboard-text.py](./clipboard-text.py) - Python 3 - Asyncio
