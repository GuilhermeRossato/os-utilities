let utilityProgramFilePath = "../clipboard-data.exe";

const child_process = require("node:child_process");

module.exports.setClipboardExecutablePath = function (filePath) {
  utilityProgramFilePath = filePath;
};

async function listClipboardFormatData() {
  const data = await new Promise((resolve, reject) => {
    const buffer = [];
    const child = child_process.spawn(utilityProgramFilePath, ["--list"], {
      stdio: ["ignore", "pipe", "pipe"],
    });
    child.stdout.on("data", (data) => buffer.push(data));
    child.stderr.on("data", (data) => buffer.push(data));
    child.on("error", reject);
    child.on("close", (code) =>
      code === 0
        ? resolve(Buffer.concat(buffer))
        : reject(new Error(`Exit code ${code}`))
    );
  });
  const text = data.toString("utf-8").trim();
  if (text.length < 2 || text[0] !== "[" || text[text.length - 1] !== "]") {
    throw new Error(`Unexpected program output: ${text}`);
  }
  /** @type {{format: number, name: string}[]} */
  const list = JSON.parse(text);
  if (list.length === 0) {
    return [];
  }
  if (typeof list[0] !== "object" || typeof list[0].format !== "number") {
    throw new Error(`Unexpected program output: ${text}`);
  }
  return list;
}

module.exports.listClipboardFormatData = listClipboardFormatData;
