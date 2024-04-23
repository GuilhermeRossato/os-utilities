# Clipboard-set

Native Windows utility to set the clipboard text data.

## Usage
```
    clipboard-set <text>            Set the clipboard text to the argument content.
    clipboard-set <text> [text...]  Set the clipboard from each argument (separated by spaces).
```

## Example

```
    Set the clipboard text to "Hello world"
        clipboard-set Hello world
```

## Node Commonjs Interface

Here are terminal commands to test this using the Node.js interface:

```bash
node -e "require('./node-cjs/getClipboardText.js').getClipboardText().then(console.log, console.error)"
node -e "require('./node-cjs/getClipboardFormat.js').getClipboardFormatList().then(console.log, console.error)"
```

