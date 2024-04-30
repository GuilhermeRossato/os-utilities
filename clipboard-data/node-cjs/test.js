const { listClipboardFormatData } = require("./listClipboardFormatData.js");
const { setClipboardFormatData } = require("./setClipboardFormatData.js");
const { getClipboardFormatData } = require("./getClipboardFormatData.js");

async function init() {
  const list = await listClipboardFormatData();
  console.log(`Clipboard list.length`, list.length);
  let newText = 'hello horld';
    for (const {format, name} of list) {
      try {
      console.log(`format`, format, `name`, name);
      const result = await getClipboardFormatData(format);
      console.log(`result`, (result instanceof Buffer) ? 'buffer' : 'unknown', result.length);
      if (format === 1) {
        console.log('Text:', JSON.stringify(result.toString()));
        
        newText = result.toString().toUppercase();
      }
    } catch (err) {
      console.log(`err`, err);
    }
  }
  console.log("Setting text clipboard to", newText);
  await setClipboardFormatData(1, newText);
}

init().catch(err => { console.log(err); process.exit(1); });