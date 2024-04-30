# Clipboard-get - CommonJS Node.js interface

This directory contains the source code of modules that exposes the parent utility program and its functionalities for Node.js scripts.

The utility is executed internally using the built-in [child_process](https://nodejs.org/api/child_process.html) module by the exported functions, which return a promise that asyncronously resolves with its parsed output.

You **must** define the executable file path of the utility program. You can either call the  `setClipboardExecutablePath` method or define it on the variable defined at the begining of the source file.

## Get Clipboard Text

```ts
getClipboardText(): Promise<string>
```

Sample code:

```js
const { getClipboardText, setClipboardExecutablePath } = require('./getClipboardText.js');

async function init() {
  setClipboardExecutablePath('../clipboard-get.exe');
  const text = await getClipboardText();
  return text;
}

init().then(console.log, console.error);
```

## Get Clipboard Format Data

```ts
getClipboardFormatList(): Promise<{format: number, name: string}[]>
getClipboardFormatData(format: number | { format: number }): Promise<Buffer>
```

Sample code:

```js
const { getClipboardFormatList, getClipboardFormatData, setClipboardExecutablePath } = require('./getClipboardFormat.js');

async function init() {
  setClipboardExecutablePath('../clipboard-get.exe');
  const list = await getClipboardFormatList();
  console.log('Format list length:', list.length);
  console.log('First format code:', list[0].format, 'name:', list[0].name);
  const buffer = await getClipboardFormatData(list[0]);
  const text = buffer.toString('utf8');
  return text;
}

init().then(console.log, console.error);
```
