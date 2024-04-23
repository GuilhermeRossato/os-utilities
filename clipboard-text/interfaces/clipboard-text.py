UTILITY_EXECUTABLE_PATH = "../clipboard-text.exe"

import subprocess
import asyncio
import os
import json

async def example():
  print('clipboard-text example - Python')

  update_utility_executable_path("../clipboard-text.exe")

  text = await get_clipboard_text()
  print(json.dumps(text))

  await set_clipboard_text("Hello world")

  text = await get_clipboard_text()
  print(json.dumps(text))

def update_utility_executable_path(exeFilePath):
  """
  Update the path for the utility executable.

  Args:
      exeFilePath (str): New path to the utility executable.
  """
  global UTILITY_EXECUTABLE_PATH
  UTILITY_EXECUTABLE_PATH = exeFilePath

async def get_clipboard_text():
  """
  Get text from the clipboard using the clipboard utility.

  Returns:
      str: Text content retrieved from the clipboard.
  """
  return await execute_clipboard_utility_process("--read", [])

async def set_clipboard_text(text):
  """
  Send the text argument to the utility to write to the clipboard.

  Args:
    text (str): Text content to set in the clipboard.

  Returns:
    str: Confirmation message.
  """
  return await execute_clipboard_utility_process("--write", [text])

async def execute_clipboard_utility_process(mode, args):
  """
  Execute the clipboard utility process with specified mode and arguments.

  Args:
    mode (str): Mode of operation ('--read' or '--write').
    args (list): Additional arguments to pass to the utility.

  Returns:
    str: Resulting output from the utility process.

  Raises:
    FileNotFoundError: If the utility executable file is not found.
    Exception: If an error occurs during execution.
  """
  if not os.path.exists(UTILITY_EXECUTABLE_PATH):
    raise FileNotFoundError(f"Executable file '{UTILITY_EXECUTABLE_PATH}' not found.")

  command = UTILITY_EXECUTABLE_PATH
  process = await asyncio.create_subprocess_exec(
    command, mode, *args,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE
  )

  stdout, stderr = await process.communicate()
  if process.returncode == 0:
    return stdout.decode("utf-8")
  else:
    raise Exception(stderr.decode("utf-8").strip() or f"Error code {process.returncode}")

# asyncio.run(example())
