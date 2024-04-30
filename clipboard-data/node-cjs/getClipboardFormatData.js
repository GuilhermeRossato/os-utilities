let utilityProgramFilePath = "../clipboard-data.exe";

const child_process = require("node:child_process");

module.exports.setClipboardExecutablePath = function (filePath) {
  utilityProgramFilePath = filePath;
};

async function getClipboardFormatData(format = 0) {
  /** @type {Buffer} */
  const raw = await new Promise((resolve, reject) => {
    const buffer = [];
    const child = child_process.spawn(
      utilityProgramFilePath,
      ["--get", format],
      { stdio: ["ignore", "pipe", "pipe"] }
    );
    child.stdout.on("data", (data) => buffer.push(data));
    child.stderr.on("data", (data) => buffer.push(data));
    child.on("error", reject);
    child.on("close", (code) =>
      code === 0
        ? resolve(Buffer.concat(buffer))
        : reject(new Error(`Exit code ${code}`))
    );
  });
  return raw;
}

module.exports.getClipboardFormatData = getClipboardFormatData;
