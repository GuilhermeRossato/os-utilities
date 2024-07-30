let UTILITY_EXECUTABLE_PATH = "../clipboard-text.exe";

import child_process from "node:child_process";

async function example() {
  console.log("clipboard-text example - Node.js (ES Modules)");

  updateUtilityExecutablePath("../clipboard-text.exe");

  let text = await getClipboardText();
  console.log(JSON.stringify(text));

  await setClipboardText("Hello world");

  text = await getClipboardText();
  console.log(JSON.stringify(text));
}

/**
 * Get text from the clipboard using the clipboard utility.
 */
export function getClipboardText() {
  return executeClipboardUtilityProcess("--read", []);
}

/**
 * Send the text argument to the utility to write to the clipboard.
 * @param {string} text
 */
export function setClipboardText(text) {
  return executeClipboardUtilityProcess("--write", [text]);
}

/**
 * Update the path for the utility executable.
 * @param {string} exeFilePath
 */
export function updateUtilityExecutablePath(exeFilePath) {
  return (UTILITY_EXECUTABLE_PATH = exeFilePath);
}

/**
 * Internal function to Execute the clipboard utility process with specified mode and arguments.
 * @param {'--read' | '--write' | '--file'} mode
 * @param  {string[]} args
 */
function executeClipboardUtilityProcess(mode, args) {
  /** @type {Promise<string>} */
  const promise = new Promise((resolve, reject) => {
    try {
      const command = UTILITY_EXECUTABLE_PATH;
      const child = child_process.spawn(command, [mode, ...args], {
        shell: false,
        stdio: ["ignore", "pipe", "pipe"],
      });
      const buffer = [];
      child.stdout.on("data", (data) => buffer.push(data));
      child.stderr.on("data", (data) => buffer.push(data));
      child.on("error", (err) =>
        reject(
          err["code"] === "ENOENT"
            ? new Error(`Could not find executable at "${command}"`, {
                cause: err,
              })
            : err
        )
      );
      child.on("exit", (exit) => {
        const text = Buffer.concat(buffer).toString();
        return exit === 0
          ? resolve(text)
          : reject(new Error(text.trim() || `Error code ${exit}`));
      });
    } catch (err) {
      reject(err);
    }
  });
  return promise;
}

// example();
