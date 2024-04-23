# OS Utilities

This repository holds self-contained utilities as individual projects used to access features (or interfaces) that are unique between operational systems.

Each folder here contains a utility project with a `readme.md` describing what they do, how they are used, and their exposed interfaces. Utilities should straightfowardly implement the necessary processes and verifications to access the features they are focused on and expose these uncommon and difficult tasks as predictable or simpler operations that can be used by other projects without polluting them with their internal details.
## Motivation

Low-level interfaces are old, configuration is error-prone, their interfaces are complex, so isolating their features in predictable parts and limiting their interfaces for simplicity will allow other projects to make use of them without having their inherent complexities, associated dependencies, and irrelevant details pollute these projects. The interface to OS-specific features often include compilation of native programs, library linking, dependencies, configurations, etc, so isolating these hard spots can turn these features into something that can be used by other projects.

The isolated utilities here are meant to provide self-contained interfaces with hidden side-effects. Their complexity is separated from the projects that will need them, and they provide a limited but simple interface to be useful.

### Utilities

- [clipboard-text](./clipboard-text/readme.md) - Read and write clipboard text data on Windows.