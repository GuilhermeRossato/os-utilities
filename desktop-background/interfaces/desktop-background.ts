let UTILITY_EXECUTABLE_PATH = "../desktop-background.exe";

import child_process from "node:child_process";
// import {Buffer} from "node:buffer";

async function example() {
  console.log("desktop-background example - Typescript");

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
 */
function updateUtilityExecutablePath(exeFilePath: string) {
  UTILITY_EXECUTABLE_PATH = exeFilePath;
}

/**
 * Get the current desktop background image file path
 */
export async function getCurrentDesktopBackgroundImage(): Promise<string> {
  const text = await executeWindowStateUtility(["--get"]);
  if (text.length <= 2 || !text.startsWith("{") || !text.endsWith("}")) {
    throw new Error(
      `Desktop background utility failed: ${JSON.stringify(text)}`
    );
  }
  const obj: { error: string } | { path: string } = JSON.parse(text);
  if ("error" in obj) {
    throw new Error(
      `Desktop background utility returned an error: ${obj.error}`
    );
  }
  if ("path" in obj) {
    return obj.path;
  }
  throw new Error(`Desktop background utility failed unexpectedly: ${text}`);
}

/**
 * Retrieves the states of children windows from the desktop.
 * @returns An array of window data for each child window.
 */
export async function setCurrentDesktopBackgroundImage(
  target: string
): Promise<string> {
  const text = await executeWindowStateUtility(["--set", target]);
  if (text.length <= 2 || !text.startsWith("{") || !text.endsWith("}")) {
    throw new Error(
      `Desktop background utility failed: ${JSON.stringify(text)}`
    );
  }
  const obj: { error: string } | { path: string } = JSON.parse(text);
  if ("error" in obj) {
    throw new Error(
      `Desktop background utility returned an error: ${obj.error}`
    );
  }
  if ("path" in obj) {
    return obj.path;
  }
  throw new Error(`Desktop background utility failed unexpectedly: ${text}`);
}

/**
 * Execute the utility process with specified arguments
 */
function executeWindowStateUtility(args: string[]): Promise<string> {
  return new Promise((resolve, reject) => {
    try {
      const command = UTILITY_EXECUTABLE_PATH;
      const child = child_process.spawn(command, args, {
        shell: false,
        stdio: ["ignore", "pipe", "pipe"],
      });
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
