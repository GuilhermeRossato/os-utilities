# OS Utilities

This repository holds self-contained utilities as individual projects used to access features (or interfaces) that are unique between operational systems.

Each folder here contains a utility project with a `readme.md` describing what they do, how they are used, and their exposed interfaces. Utilities should straightfowardly implement the necessary processes and verifications to access the features they are focused on and expose these uncommon and difficult tasks as predictable or simpler operations that can be used by other projects without polluting them with their internal details.

## Utilities Created

- [desktop-background](./desktop-background/readme.md) - Get or set the current display background image file.
- [clipboard-text](./clipboard-text/readme.md) - Read and write clipboard text data on Windows.
- [clipboard-data](./clipboard-data/readme.md) - Read, write, and list clipboard data of specific formats on Windows.
- [window-state](./window-state/readme.md) - Interact with the properties of window elements.

## Motivation

Configuration of low-level interfaces is error-prone, their interfaces are complex, and their features require lengthy experimentation to get right. Using a divide-and-conquer strategy and isolating their behaviors, requirements, and dependencies in minimal and predictable programs help us understand, operate, and re-use them in other projects.

The interface to OS-specific features often include compilation of native programs, library linking, dependencies, configurations, etc, isolating these difficult processes in self-contained minimal programs make these features available to be operated from other projects without having their associated dependencies and irrelevant complexitities pollute these projects.

Native bindings offered by most high-level languages can interface with C/C++ programs, but these depend on having the right version for everything: From the program that interprets the language, to the installed compiler, cmake-related programs, and the right package version, which is not always up to date. It is indeed a good day when an install command that depends on native bindings just works: It's never that simple.

The isolated utilities here are meant to provide self-contained interfaces that abstract away side-effects and separate its complexity from the projects that will need them.
