let UTILITY_EXECUTABLE_PATH = "../desktop-background.exe";

const child_process = require("node:child_process");

async function example() {
  console.log("desktop-background example - Node.js (CommonJS)");

  updateUtilityExecutablePath("../desktop-background.exe");

  let p = await getCurrentDesktopBackgroundImage();
  console.log("Current desktop background image:");
  console.log(p);
  p = await setCurrentDesktopBackgroundImage(p);
  console.log("Updated the desktop background image:");
  console.log(p);
}

/**
 * Update the path for the utility executable.
 * @param {string} exeFilePath
 */
function updateUtilityExecutablePath(exeFilePath) {
  UTILITY_EXECUTABLE_PATH = exeFilePath;
}

module.exports.updateUtilityExecutablePath = updateUtilityExecutablePath;

/**
 * Get the current desktop background image file path
 * @returns {Promise<string>}
 */
async function getCurrentDesktopBackgroundImage() {
  const text = await executeWindowStateUtility(["--get"]);
  if (text.length <= 2 || !text.startsWith("{") || !text.endsWith("}")) {
    throw new Error(`Desktop background utility failed: ${JSON.stringify(text)}`);
  }
  /** @type {{"error": string} | {"path": string}} */
  const list = JSON.parse(text);
  if (list.error) {
    throw new Error(`Desktop background utility returned an error: ${list.error}`);
  }
  if (list.path) {
    return list.path;
  }
  throw new Error(`Desktop background utility failed unexpectedly with output: ${text}`);
}

module.exports.getCurrentDesktopBackgroundImage = getCurrentDesktopBackgroundImage;

/**
 * Set the desktop background image to a target image file path
 * @param {string} target
 * @returns {Promise<string>}
 */
async function setCurrentDesktopBackgroundImage(target) {
  const text = await executeWindowStateUtility(["--set", target]);
  if (text.length <= 2 || !text.startsWith("{") || !text.endsWith("}")) {
    throw new Error(`Desktop background utility failed: ${JSON.stringify(text)}`);
  }
  /** @type {{"error": string} | {"path": string}} */
  const list = JSON.parse(text);
  if (list.error) {
    throw new Error(`Desktop background utility returned an error: ${list.error}`);
  }
  if (list.path) {
    return list.path;
  }
  throw new Error(`Desktop background utility failed unexpectedly with output: ${text}`);
}

module.exports.setCurrentDesktopBackgroundImage = setCurrentDesktopBackgroundImage;

/**
 * Execute the utility process with specified arguments
 * @param {string[]} args
 */
function executeWindowStateUtility(args) {
  /** @type {Promise<string>} */
  const promise = new Promise((resolve, reject) => {
    try {
      const command = UTILITY_EXECUTABLE_PATH;
      const child = child_process.spawn(
        command,
        args.map((a, i) =>
          typeof a === "object" && typeof a.handle === "number"
            ? a.handle.toString()
            : a.toString()
        ),
        {
          shell: false,
          stdio: ["ignore", "pipe", "pipe"],
        }
      );
      const chunks = [];
      child.stdout.on("data", (data) => chunks.push(data));
      child.stderr.on("data", (data) => chunks.push(data));
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
        const text = Buffer.concat(chunks).toString("utf8").trim();
        return exit === 0
          ? resolve(text)
          : reject(new Error(text || `Error code ${exit}`));
      });
    } catch (err) {
      reject(err);
    }
  });
  return promise;
}

example();
