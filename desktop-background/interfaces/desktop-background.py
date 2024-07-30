UTILITY_EXECUTABLE_PATH = "../desktop-background.exe"

import subprocess
import asyncio
import os
import json

async def example():
  print("desktop_background example - Python")

  update_utility_executable_path("../desktop-background.exe")

  p = await get_current_desktop_background_image()
  print("Current desktop background image:")
  print(p)
  p = await set_current_desktop_background_image(p)
  print("Updated the desktop background image:")
  print(p)
  

def update_utility_executable_path(exeFilePath):
  """
  Update the path for the utility executable.
  """
  global UTILITY_EXECUTABLE_PATH
  UTILITY_EXECUTABLE_PATH = exeFilePath


async def get_current_desktop_background_image():
  """
  Retrieves the state of the foreground window.
  """
  text = await execute_window_state_utility(["--get"])
  if len(text) <= 2 or not text.startswith("{") or not text.endswith("}"):
    raise ValueError(f"Desktop background utility failed unexpectedly: {json.dumps(text)}")
  obj = json.loads(text)
  if not obj or not "path" in obj:
    raise ValueError(f"Desktop background utility failed unexpectedly: {json.dumps(text)}")
  return obj.path

async def set_current_desktop_background_image(target):
  """
  Retrieves the state of the foreground window.
  """
  text = await execute_window_state_utility(["--set", target])
  if len(text) <= 2 or not text.startswith("{") or not text.endswith("}"):
    raise ValueError(f"Desktop background utility failed unexpectedly: {json.dumps(text)}")
  obj = json.loads(text)
  if not obj or not "path" in obj:
    raise ValueError(f"Desktop background utility failed unexpectedly: {json.dumps(text)}")
  return obj.path


async def execute_window_state_utility(args):
  if not isinstance(UTILITY_EXECUTABLE_PATH, str):
    raise TypeError("Executable file path must be a string")

  if not os.path.exists(UTILITY_EXECUTABLE_PATH):
    raise FileNotFoundError(
      f"Executable file '{UTILITY_EXECUTABLE_PATH}' not found."
    )
    
  # Convert number and boolean arguments to strings
  args = [str(arg) if not isinstance(arg, (str, bytes)) else arg for arg in args]

  if not all(isinstance(arg, str) for arg in args):
    raise TypeError("Arguments must be a list of strings")


  command = UTILITY_EXECUTABLE_PATH
  process = await asyncio.create_subprocess_exec(
    command, *args, stdout=subprocess.PIPE, stderr=subprocess.PIPE
  )

  stdout, stderr = await process.communicate()

  if process.returncode == 0:
    return stdout.decode("utf-8")

  raise Exception(
    stderr.decode("utf-8").strip() or f"Error code {process.returncode}"
  )

# asyncio.run(example())
