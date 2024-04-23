let UTILITY_EXECUTABLE_PATH = '../clipboard-text.exe';

const child_process = require("node:child_process");

async function example() {
  console.log('clipboard-text example - Node.js (CommonJS)');

  updateUtilityExecutablePath('../clipboard-text.exe');

  let text = await getClipboardText();
  console.log(JSON.stringify(text));

  await setClipboardText("Hello world");

  text = await getClipboardText();
  console.log(JSON.stringify(text));
}

/**
 * Get text from the clipboard using the clipboard utility.
 */
function getClipboardText() {
  return executeClipboardUtilityProcess('--read', []);
}

exports.getClipboardText = getClipboardText;

/**
 * Send the text argument to the utility to write to the clipboard.
 * @param {string} text
 */
async function setClipboardText(text) {
  await executeClipboardUtilityProcess('--write', [text]);
}

exports.setClipboardText = setClipboardText;

/**
 * Update the path for the utility executable.
 * @param {string} exeFilePath 
 */
function updateUtilityExecutablePath(exeFilePath) {
  UTILITY_EXECUTABLE_PATH = exeFilePath;
}

exports.updateUtilityExecutablePath = updateUtilityExecutablePath;

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
      const child = child_process.spawn(
        command,
        [mode, ...args],
        {
          shell: false,
          stdio: ['ignore', 'pipe', 'pipe']
        }
      );
      const chunks = [];
      child.stdout.on("data", (data) => chunks.push(data));
      child.stderr.on("data", (data) => chunks.push(data));
      child.on("error", (err) => reject(err['code'] === 'ENOENT' ? new Error(`Could not find executable at "${command}"`, {cause: err}) : err));
      child.on("exit", (exit) => {
        const text = Buffer.concat(chunks).toString('utf8');
        return exit === 0 ? resolve(text) : reject(new Error(text.trim() || `Error code ${exit}`));
      });
    } catch (err) {
      reject(err);
    }
  });
  return promise;
};


// example();