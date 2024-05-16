let UTILITY_EXECUTABLE_PATH = "../window-state.exe";

const child_process = require("node:child_process");

async function example() {
  console.log("window-state example - Node.js (CommonJS)");

  updateUtilityExecutablePath("../window-state.exe");

  const fore = await getForegroundWindowState();
  console.log("Foreground");
  console.log(fore);

  if (fore.sibling) {
    const sibling = await getWindowState(fore.sibling);
    console.log("Foreground sibling");
    console.log(sibling);
  }

  const list = await getDesktopChildrenStates();
  console.log("Desktop", JSON.stringify(list.slice(0, 2)));
  console.log("Window Count:", list.length);

  console.log("Minimizing foreground in 1 second...");
  await new Promise((resolve) => setTimeout(resolve, 1000));
  await setWindowState(await getForegroundWindowState(), { minimize: true });
}

/**
 * @typedef {object} WindowState - Represents detailed information about a window.
 * @property {number} handle - The handle of the window.
 * @property {string} [title] - The title of the window.
 * @property {string} [module] - The module path associated with the window.
 * @property {string} [executable] - The executable path related to the window.
 * @property {string} [classname] - The class name of the window.
 * @property {number} parent - The handle of the parent window or 0 if there is no parent.
 * @property {number} [sibling] - The handle of the next sibling window if one exists.
 * @property {number} [child] - The handle of the first child of the window if it has any.
 * @property {number} pid - The process ID associated with the window.
 * @property {number} thread - The thread ID related to the window.
 * @property {number} style - The numeric representation of the style attributes of the window (from GWL_STYLE).
 * @property {number} exstyle - The numeric representation of the extended style attributes of the window (from GWL_EXSTYLE).
 * @property {boolean} visible - Indicates if the window is visible.
 * @property {boolean} unicode - Indicates if the window supports unicode.
 * @property {boolean} [popup] - Indicates if the window is a popup.
 * @property {boolean} [bordered] - Indicates if the window has borders.
 * @property {boolean} [scrollable] - Indicates if the window is scrollable.
 * @property {boolean} [contained] - Indicates if the window is contained within another window.
 * @property {boolean} [bordered] - Indicates if the window has borders.
 * @property {boolean} [scrollable] - Indicates if the window is scrollable.
 * @property {boolean} [minimized] - Indicates if the window is minimized.
 * @property {boolean} [topmost] - Indicates if the window is at the top layer.
 * @property {boolean} [transparent] - Indicates if the window has transparency.
 * @property {number} [top] - The top position of the window in pixels.
 * @property {number} [right] - The right position of the window.
 * @property {number} [bottom] - The bottom position of the window.
 * @property {number} [left] - The left position of the window.
 */

/**
 * Update the path for the utility executable.
 * @param {string} exeFilePath
 */
function updateUtilityExecutablePath(exeFilePath) {
  UTILITY_EXECUTABLE_PATH = exeFilePath;
}

/**
 * Retrieves the state of the foreground window.
 * @returns {Promise<WindowState>} The data of the foreground window.
 */
async function getForegroundWindowState() {
  const text = await executeWindowStateUtility(["--foreground"]);
  if (text.length <= 2 || !text.startsWith("[") || !text.endsWith("]")) {
    throw new Error(`Window state failed: ${JSON.stringify(text)}`);
  }
  /** @type {WindowState[]} */
  const list = JSON.parse(text);
  if (!list.length || !list[0]) {
    throw new Error("Window state list is empty");
  }
  if (!(list instanceof Array) || typeof list[0] !== "object") {
    throw new Error(`Window state list is invalid: ${JSON.stringify(list)}`);
  }
  const problems = list.filter(
    (a) => !a || typeof a.handle !== "number" || typeof a.pid !== "number"
  );
  if (problems.length) {
    throw new Error(
      `Unexpected response with invalid elements: ${JSON.stringify(problems)}`
    );
  }
  return list[0];
}

module.exports.getForegroundWindowState = getForegroundWindowState;

/**
 * Retrieves the states of children windows from the desktop.
 * @returns {Promise<WindowState[]>} An array of window data for each child window.
 */
async function getDesktopChildrenStates() {
  const text = await executeWindowStateUtility(["--desktop"]);
  if (text.length <= 2 || !text.startsWith("[") || !text.endsWith("]")) {
    throw new Error(`Window state failed: ${JSON.stringify(text)}`);
  }
  /** @type {WindowState[]} */
  const list = JSON.parse(text);
  if (!list.length || !list[0]) {
    throw new Error("Window state list is empty");
  }
  if (!(list instanceof Array) || typeof list[0] !== "object") {
    throw new Error(`Window state list is invalid: ${JSON.stringify(list)}`);
  }
  const problems = list.filter(
    (a) => !a || typeof a.handle !== "number" || typeof a.pid !== "number"
  );
  if (problems.length) {
    throw new Error(
      `Unexpected response with invalid elements: ${JSON.stringify(problems)}`
    );
  }
  return list;
}

module.exports.getDesktopChildrenStates = getDesktopChildrenStates;

/**
 * Retrieves the state of a specific window by its handle.
 * @param {number} handle - The handle of the window to retrieve data for.
 * @returns {Promise<WindowState>} The data of the specified window.
 */
async function getWindowState(handle) {
  const text = await executeWindowStateUtility(["--handle", handle]);
  if (text.length <= 2 || !text.startsWith("[") || !text.endsWith("]")) {
    throw new Error(`Window state failed: ${JSON.stringify(text)}`);
  }
  /** @type {WindowState[]} */
  const list = JSON.parse(text);
  const problems = list.filter(
    (a) => !a || typeof a.handle !== "number" || typeof a.pid !== "number"
  );
  if (problems.length) {
    throw new Error(
      `Unexpected response with invalid elements: ${JSON.stringify(problems)}`
    );
  }
  return list[0];
}

module.exports.getWindowState = getWindowState;

/**
 * Sets the state of a window based on the provided configuration.
 * @param {number | {handle: number}} handle - The handle of the window to set state for.
 * @param {object} config - Configuration object specifying the window state changes.
 */
async function setWindowState(
  handle,
  config = {
    left: -1,
    top: -1,
    x: -1,
    y: -1,
    width: -1,
    height: -1,
    foreground: null,
    setTop: null,
    setTopMost: null,
    minimize: null,
    maximize: null,
    visible: null,
    show: null,
    hide: null,
  }
) {
  const args = [
    "--handle",
    typeof handle === "object" && typeof handle.handle === "number"
      ? handle.handle
      : handle,
  ];
  const xCoord = config.left ?? config.x;
  const yCoord = config.top ?? config.y;
  if (
    typeof xCoord === "number" &&
    typeof yCoord === "number" &&
    xCoord >= 0 &&
    yCoord >= 0
  ) {
    args.push("--move", xCoord, yCoord);
  }
  if (
    typeof config.width === "number" &&
    typeof config.height === "number" &&
    config.width > 0 &&
    config.height > 0
  ) {
    args.push("--resize", config.width, config.height);
  }
  if (typeof config.visible === "boolean") {
    args.push(config.visible ? "--show" : "--hide");
  } else if (config.show === true) {
    args.push("--show");
  } else if (
    config.hide === true ||
    (config.show === false && config.hide !== false)
  ) {
    args.push("--hide");
  }
  if (config.foreground === true) {
    args.push("--set-foreground");
  }
  if (config.setTop === true) {
    args.push("--set-top");
  }
  if (config.setTopMost === true) {
    args.push("--set-top-most");
  }
  if (config.maximize === true) {
    args.push("--maximize");
  } else if (config.minimize === true) {
    args.push("--minimize");
  }
  const text = await executeWindowStateUtility(args);
  if (text.length === 0) {
    return;
  }
  throw new Error(`Window state returned: ${JSON.stringify(text)}`);
}

module.exports.setWindowState = setWindowState;

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

// example();
