UTILITY_EXECUTABLE_PATH = "../window-state.exe"

import subprocess
import asyncio
import os
import json


async def example():
  print("window-state example - Python")

  update_utility_executable_path("../window-state.exe")

  fore = await get_foreground_window_state()
  print("Foreground")
  print(json.dumps(fore.__dict__))

  if "sibling" in fore:
    sibling = await get_window_state(fore.sibling)
    print("Foreground sibling")
    print(sibling.to_json())

  list = await get_desktop_children_states()
  print("Desktop", list[0].to_json(), list[1].to_json())
  print("Window Count:", len(list))

  print("Minimizing foreground in 1 second...")
  await asyncio.sleep(1)
  fore = await get_foreground_window_state()
  await set_window_state(fore, minimize=True)


def update_utility_executable_path(exeFilePath):
  """
  Update the path for the utility executable.
  """
  global UTILITY_EXECUTABLE_PATH
  UTILITY_EXECUTABLE_PATH = exeFilePath


async def get_foreground_window_state():
  """
  Retrieves the state of the foreground window.
  """
  text = await execute_window_state_utility(["--foreground"])
  if len(text) <= 2 or not text.startswith("[") or not text.endswith("]"):
    raise ValueError(f"Window state failed: {json.dumps(text)}")
  list_data = json.loads(text)
  if not list_data or not list_data[0]:
    raise ValueError("Window state list is empty")
  if not isinstance(list_data, list) or not isinstance(list_data[0], dict):
    raise ValueError(f"Window state list is invalid: {json.dumps(list_data)}")
  problems = [
    a
    for a in list_data
    if not a
    or not isinstance(a.get("handle"), int)
    or not isinstance(a.get("pid"), int)
  ]
  if problems:
    raise ValueError(
      f"Unexpected response with invalid elements: {json.dumps(problems)}"
    )
  window_data = list_data[0]
  return WindowState(**window_data)


async def get_desktop_children_states():
  text = await execute_window_state_utility(["--desktop"])
  if len(text) <= 2 or not text.startswith("[") or not text.endswith("]"):
    raise ValueError(f"Window state failed: {json.dumps(text)}")
  list_data = json.loads(text)
  if not list_data or not list_data[0]:
    raise ValueError("Window state list is empty")
  if not isinstance(list_data, list) or not isinstance(list_data[0], dict):
    raise ValueError(f"Window state list is invalid: {json.dumps(list_data)}")
  problems = [
    a
    for a in list_data
    if not a
    or not isinstance(a.get("handle"), int)
    or not isinstance(a.get("pid"), int)
  ]
  if problems:
    raise ValueError(
      f"Unexpected response with invalid elements: {json.dumps(problems)}"
    )
  window_states = []
  for window_data in list_data:
    window_state = WindowState(**window_data)
    window_states.append(window_state)
  return window_states


async def get_window_state(handle):
  text = await execute_window_state_utility(["--handle", handle])
  if len(text) <= 2 or not text.startswith("[") or not text.endswith("]"):
    raise ValueError(f"Window state failed: {json.dumps(text)}")
  list_data = json.loads(text)
  problems = [
    a
    for a in list_data
    if not a
    or not isinstance(a.get("handle"), int)
    or not isinstance(a.get("pid"), int)
  ]
  if problems:
    raise ValueError(
      f"Unexpected response with invalid elements: {json.dumps(problems)}"
    )
  window_data = list_data[0]
  return WindowState(**window_data)


async def set_window_state(
  handle,
  left=None,
  top=None,
  x=None,
  y=None,
  width=None,
  height=None,
  visible=None,
  show=None,
  hide=None,
  maximize=None,
  minimize=None,
  foreground=None,
  set_top_most=None,
  set_top=None,
):
  """
  Sets the state of a window based on the provided arguments.
  """
  args = [
    "--handle",
    handle.handle
    if (isinstance(handle, dict) or isinstance(handle, WindowState))
    and "handle" in handle
    else handle,
  ]
  if left is not None and top is not None and left >= 0 and top >= 0:
    args.extend(["--move", left, top])
  elif x is not None and y is not None and x >= 0 and y >= 0:
    args.extend(["--move", x, y])
  if width is not None and height is not None and width >= 0 and height >= 0:
    args.extend(["--resize", width, height])
  if visible is not None:
    args.append("--show" if visible else "--hide")
  elif show is True:
    args.append("--show")
  elif (hide is True) or (show is False and hide is None):
    args.append("--hide")
  if foreground is True:
    args.append("--set-foreground")
  if set_top is True:
    args.append("--set-top")
  if set_top_most is True:
    args.append("--set-top-most")
  if maximize is True:
    args.append("--maximize")
  elif minimize is True:
    args.append("--minimize")
  text = await execute_window_state_utility(args)
  if not text:
    return
  raise ValueError(f"Window state returned: {json.dumps(text)}")

class WindowState:
  """
  Represents detailed information about a window.

  Attributes:
    handle (int): The handle of the window.
    title (str, optional): The title of the window.
    module (str, optional): The module path associated with the window.
    executable (str, optional): The executable path related to the window.
    classname (str, optional): The class name of the window.
    parent (int): The handle of the parent window or 0 if there is no parent.
    sibling (int, optional): The handle of the next sibling window if one exists.
    child (int, optional): The handle of the first child of the window if it has any.
    pid (int): The process ID associated with the window.
    thread (int): The thread ID related to the window.
    style (int): The numeric representation of the style attributes of the window (from GWL_STYLE).
    exstyle (int): The numeric representation of the extended style attributes of the window (from GWL_EXSTYLE).
    visible (bool): Indicates if the window is visible.
    unicode (bool): Indicates if the window supports unicode.
    popup (bool, optional): Indicates if the window is a popup.
    bordered (bool, optional): Indicates if the window has borders.
    scrollable (bool, optional): Indicates if the window is scrollable.
    contained (bool, optional): Indicates if the window is contained within another window.
    minimized (bool, optional): Indicates if the window is minimized.
    topmost (bool, optional): Indicates if the window is at the top layer.
    transparent (bool, optional): Indicates if the window has transparency.
    top (int, optional): The top position of the window in pixels.
    right (int, optional): The right position of the window.
    bottom (int, optional): The bottom position of the window.
    left (int, optional): The left position of the window.
  """

  def __init__(
    self,
    handle,
    title=None,
    module=None,
    executable=None,
    classname=None,
    parent=0,
    sibling=None,
    child=None,
    pid=None,
    thread=None,
    style=None,
    exstyle=None,
    visible=False,
    unicode=False,
    popup=None,
    bordered=None,
    scrollable=None,
    contained=None,
    minimized=None,
    topmost=None,
    transparent=None,
    top=None,
    right=None,
    bottom=None,
    left=None,
  ):
    self.handle = handle
    self.title = title
    self.module = module
    self.executable = executable
    self.classname = classname
    self.parent = parent
    self.sibling = sibling
    self.child = child
    self.pid = pid
    self.thread = thread
    self.style = style
    self.exstyle = exstyle
    self.visible = visible
    self.unicode = unicode
    self.popup = popup
    self.bordered = bordered
    self.scrollable = scrollable
    self.contained = contained
    self.minimized = minimized
    self.topmost = topmost
    self.transparent = transparent
    self.top = top
    self.right = right
    self.bottom = bottom
    self.left = left
    self.__dict__ = self.gen_dict()

  def __iter__(self):
    for key in self.__dict__.keys():
      yield key

  def __getitem__(self, key):
    return getattr(self, key)

  def __setitem__(self, key, value):
    setattr(self, key, value)

  def keys(self):
    return self.__dict__.keys()

  def items(self):
    return [(key, getattr(self, key)) for key in self.__dict__.keys()]

  def __len__(self):
    return len(self.__dict__)

  def __contains__(self, item):
    return hasattr(self, item)

  def to_dict(self):
    return self.__dict__

  def to_json(self):
    return json.dumps(self.to_dict())
  
  def gen_dict(self):
    return {
      "handle": self.handle,
      "title": self.title,
      "module": self.module,
      "executable": self.executable,
      "classname": self.classname,
      "parent": self.parent,
      "sibling": self.sibling,
      "child": self.child,
      "pid": self.pid,
      "thread": self.thread,
      "style": self.style,
      "exstyle": self.exstyle,
      "visible": self.visible,
      "unicode": self.unicode,
      "popup": self.popup,
      "bordered": self.bordered,
      "scrollable": self.scrollable,
      "contained": self.contained,
      "minimized": self.minimized,
      "topmost": self.topmost,
      "transparent": self.transparent,
      "top": self.top,
      "right": self.right,
      "bottom": self.bottom,
      "left": self.left,
    }


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


asyncio.run(example())
