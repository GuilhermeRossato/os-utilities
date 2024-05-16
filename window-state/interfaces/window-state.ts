let UTILITY_EXECUTABLE_PATH = "../window-state.exe";

import child_process from "node:child_process";

async function example() {
  console.log("window-state example - Typescript");

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
 * Represents detailed information about a window.
 */
export interface WindowState {
  /** The handle of the window. */
  handle: number;

  /** The title of the window. */
  title?: string;

  /** The top position of the window in pixels. */
  top?: number;

  /** The right position of the window. */
  right?: number;

  /** The bottom position of the window. */
  bottom?: number;

  /** The left position of the window. */
  left?: number;

  /** The module path associated with the window. */
  module?: string;

  /** The executable path related to the window. */
  executable?: string;

  /** The class name of the window. */
  classname?: string;

  /** The handle of the parent window or 0 if there is no parent. */
  parent: number;

  /** The handle of the next sibling window if one exists. */
  sibling?: number;

  /** The handle of the first child of the window if it has any. */
  child?: number;

  /** The process ID associated with the window. */
  pid: number;

  /** The thread ID related to the window. */
  thread: number;

  /** The numeric representation of the style attributes of the window (from GWL_STYLE). */
  style: number;

  /** The numeric representation of the extended style attributes of the window (from GWL_EXSTYLE). */
  exstyle: number;

  /** Indicates if the window is visible. */
  visible: boolean;

  /** Indicates if the window supports unicode. */
  unicode: boolean;

  /** True if the window is a popup. */
  popup?: boolean;

  /** True if the window has borders. */
  bordered?: boolean;

  /** True if the window is scrollable. */
  scrollable?: boolean;

  /** True if the window is contained within another window. */
  contained?: boolean;

  /** True if the window is minimized. */
  minimized?: boolean;

  /** True if the window is at the top layer. */
  topmost?: boolean;

  /** True if the window is transparent. */
  transparent?: boolean;
}

export type HandleLike = number | string | WindowState | { handle: number };

/**
 * Update the path for the utility executable.
 */
function updateUtilityExecutablePath(exeFilePath: string) {
  UTILITY_EXECUTABLE_PATH = exeFilePath;
}

/**
 * Retrieves the state of the foreground window.
 */
export async function getForegroundWindowState(): Promise<WindowState> {
  const text = await executeWindowStateUtility(["--foreground"]);
  if (text.length <= 2 || !text.startsWith("[") || !text.endsWith("]")) {
    throw new Error(`Window state failed: ${JSON.stringify(text)}`);
  }
  const list: WindowState[] = JSON.parse(text);
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

/**
 * Retrieves the states of children windows from the desktop.
 * @returns An array of window data for each child window.
 */
export async function getDesktopChildrenStates(): Promise<WindowState[]> {
  const text = await executeWindowStateUtility(["--desktop"]);
  if (text.length <= 2 || !text.startsWith("[") || !text.endsWith("]")) {
    throw new Error(`Window state failed: ${JSON.stringify(text)}`);
  }
  const list: WindowState[] = JSON.parse(text);
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

/**
 * Retrieves the state of a specific window by its handle.
 */
export async function getWindowState(handle: HandleLike): Promise<WindowState> {
  const text = await executeWindowStateUtility(["--handle", handle]);
  if (text.length <= 2 || !text.startsWith("[") || !text.endsWith("]")) {
    throw new Error(`Window state failed: ${JSON.stringify(text)}`);
  }
  const list: WindowState[] = JSON.parse(text);
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

/**
 * Sets the state of a window based on the provided configuration.
 */
export async function setWindowState(
  handle: HandleLike,
  config: Partial<{
    left: number;
    top: number;
    x: number;
    y: number;
    width: number;
    height: number;
    foreground: boolean;
    setTop: boolean;
    setTopMost: boolean;
    minimize: boolean;
    maximize: boolean;
    visible: boolean;
    show: boolean;
    hide: boolean;
  }> = {}
) {
  const args: (string | number)[] = ["--handle"];
  if (typeof handle === "number" || typeof handle === "string") {
    args.push(handle.toString());
  }
  if (typeof handle === "object" && typeof handle.handle === "number") {
    args.push(handle.handle.toString());
  }
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

/**
 * Execute the utility process with specified arguments
 */
function executeWindowStateUtility(args: HandleLike[]): Promise<string> {
  return new Promise((resolve, reject) => {
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
      const chunks: Buffer[] = [];
      child.stdout.on("data", (data) => chunks.push(data));
      child.stderr.on("data", (data) => chunks.push(data));
      child.on("error", (err: any) =>
        reject(
          err.code === "ENOENT"
            ? new Error(`Could not find executable at "${command}"`)
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
}

// example();
