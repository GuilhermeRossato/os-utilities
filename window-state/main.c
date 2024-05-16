#include "windows.h"
#include "stdio.h"
#include <psapi.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#pragma comment(lib, "User32.lib")

#define verbose 0

void printHelp()
{
  printf("window-state - Utility to interact with window states\n");
  printf("\n");
  printf("Usage:\n");
  printf("\twindow-state [...filters] [...operations]\n");
  printf("\n");
  printf("Filters:\n");
  printf("\n");
  printf("\t\t--foreground         Select the focused window currently active\n");
  printf("\t\t--handle <handle>    Select a window by its numeric handle id.\n");
  printf("\t\t--desktop            Select all children from the top-level desktop window object.\n");
  printf("\t\t--parent <handle>    Select all children of a specific window.\n");
  // Features not implemented
  // printf("\t\t--pid <pid>          Filter windows by a process.\n");
  // printf("\t\t--title <substring>  Filter windows with titles that includes a substring.\n");
  // printf("\t\t--style <code>       Filter by matching style code.\n");
  // printf("\t\t--exstyle <code>     Filter by matching extended style code.\n");
  // printf("\t\t--message            Filter message-only windows.\n");
  printf("\n");
  printf("Operations:\n");
  printf("\n");
  printf("\t\t--move <x> <y>       Move matching windows to a specific position\n");
  printf("\t\t--size <w> <h>       Resize matching windows to a specified size.\n");
  printf("\t\t--show               Show matching windows.\n");
  printf("\t\t--hide               Hide matching windows.\n");
  printf("\t\t--maximize           Maximize matching windows.\n");
  printf("\t\t--minimize           Minimize matching windows.\n");
  printf("\t\t--set-foreground     Set the first match as the focused window.\n");
  printf("\t\t--set-top            Bring the first matching window to top.\n");
  printf("\t\t--set-top-most       Bring the first matching window to the top-most layer.\n");
  // Features not implemented
  // printf("\t\t--long <i>           Read a window long value from each matching window.\n");
  // printf("\t\t--word <i>           Read a window word value from each matching window.\n");
}

#define BUFFER_SIZE 1024 * 1024
#define MIDDLE_BUFFER_SIZE MAX_PATH * 2
#define SMALL_BUFFER_SIZE 64

TCHAR exec_file_path[MIDDLE_BUFFER_SIZE];
size_t exec_file_path_length;

char buffer[BUFFER_SIZE];
size_t buffer_length = 0;

char module[MIDDLE_BUFFER_SIZE];
size_t module_length = 0;

char title[MIDDLE_BUFFER_SIZE];
size_t title_length = 0;

char class[MIDDLE_BUFFER_SIZE];
size_t class_length = 0;

RECT rect;

int is_filter_foreground = 0;
int is_filter_desktop = 0;
int is_filter_message = 0;
char filter_title[MIDDLE_BUFFER_SIZE];
size_t filter_title_length = 0;
int is_filter_title = 0;

int is_filter_handle = 0;
int64_t filter_handle_list[SMALL_BUFFER_SIZE];
int filter_handle_list_size = 0;
int is_filter_parent = 0;
int64_t filter_parent = 0;
int is_filter_pid = 0;
int64_t filter_pid = 0;
int is_filter_style = 0;
int64_t filter_style = 0;
int is_filter_exstyle = 0;
int64_t filter_exstyle = 0;

int is_action_set_foreground = 0;
int is_action_bring_to_top = 0;
int is_action_move = 0;
int action_x = 0;
int action_y = 0;
int is_action_size = 0;
int action_w = 0;
int action_h = 0;
int is_action_show = 0;
int is_action_hide = 0;
int is_action_maximize = 0;
int is_action_minimize = 0;

HWND last_next;

size_t makeWindowJson(char *buffer, size_t buffer_size, HWND h);
size_t putWindowJson(char *buffer, size_t buffer_size, HWND h, char *title, size_t title_size, char *module, size_t module_size, char *exec, size_t exec_length, char *class, size_t class_size, HWND parent, HWND next, HWND child, DWORD pid, DWORD thread, LONG style, int64_t exstyle, int is_unicode, int is_visible, int is_popup, int is_contained, int is_bordered, int is_scrollable, int is_visible_alt, int is_minimized, int is_topmost, int is_transparent, RECT *rect);

int checkHasActions()
{
  return (is_action_set_foreground != 0 || is_action_bring_to_top != 0 || is_action_move != 0 || is_action_size != 0 || is_action_show != 0 || is_action_hide != 0 || is_action_maximize != 0 || is_action_minimize != 0);
}

int applyActions(HWND handle)
{
  int error_line = 0;

  if (handle != NULL && is_action_set_foreground != 0)
  {
    if (verbose)
      printf("[Verbose] Executing SetForegroundWindow on %" PRId64 "\n", (int64_t)handle);
    if (0 == SetForegroundWindow(handle))
    {
      printf("Warning: SetForegroundWindow failed for %" PRId64 "\n", (int64_t)handle);
      error_line = 118;
    }
    else if (is_action_set_foreground != 0)
      is_action_set_foreground = 0;
  }

  int show_arg = (is_action_show != 0 ? SW_SHOW : 0) | (is_action_maximize != 0 ? SW_MAXIMIZE : 0) | (is_action_hide != 0 ? SW_HIDE : 0) | (is_action_minimize != 0 ? SW_MINIMIZE : 0);

  if (handle != NULL && show_arg != 0)
  {
    if (verbose)
      printf("[Verbose] Executing ShowWindow on %" PRId64 " with %c%c%c%c: %" PRId64 "\n", (int64_t)handle, (is_action_show != 0 ? 'S' : ' '), (is_action_maximize != 0 ? 'M' : ' '), (is_action_hide != 0 ? 'H' : ' '), (is_action_minimize != 0 ? 'm' : ' '), (int64_t)show_arg);
    ShowWindow(handle, show_arg);
  }

  if (handle != NULL && (is_action_bring_to_top != 0 || is_action_move != 0 || is_action_size != 0))
  {
    HWND insert_arg = is_action_bring_to_top == 2 ? HWND_TOPMOST : is_action_bring_to_top == 1 ? HWND_TOP
                                                                                               : NULL;
    UINT flags_arg;
    if (!is_action_move && !is_action_size)
      flags_arg = SWP_NOMOVE | SWP_NOSIZE;
    else if (!is_action_move && is_action_size)
      flags_arg = SWP_NOMOVE;
    else if (is_action_move && !is_action_size)
      flags_arg = SWP_NOSIZE;
    else if (is_action_move && is_action_size)
      flags_arg = 0;
    if (verbose)
      printf("[Verbose] Executing SetWindowPos on %" PRId64 "\n", (int64_t)handle);
    if (0 == SetWindowPos(handle, insert_arg, action_x, action_y, action_w, action_h, flags_arg))
    {
      printf("Warning: SetWindowPos failed for %" PRId64 "\n", (int64_t)handle);
      error_line = 155;
    }
    else if (is_action_bring_to_top != 0)
      is_action_bring_to_top = 0;
  }

  return error_line;
}

int startProgram()
{
  HWND handle;
  int is_win;
  int i;
  int err;
  int isActionMode = checkHasActions();

  if (is_filter_handle)
  {
    is_win = filter_handle_list[0] != 0 && IsWindow((HWND)filter_handle_list[0]);
    if (verbose)
      printf("[Verbose] %s handle source: \"filter_handle[0]\": %" PRId64 " (%s)\n", filter_handle_list_size == 1 ? "Single" : "Multiple", (int64_t)filter_handle_list[0], is_win ? "valid window" : "invalid window");
    // Verify handle list
    for (i = 0; i < filter_handle_list_size; i++)
    {
      handle = (HWND)filter_handle_list[i];
      is_win = handle != NULL && handle != 0 && IsWindow(handle);
      if (!is_win)
      {
        printf("Error: Target handle %" PRId64 " was not found\n", (int64_t)handle);
        return 170;
      }
    }
    if (isActionMode)
    {
      for (i = 0; i < filter_handle_list_size; i++)
      {
        handle = (HWND)filter_handle_list[i];
        err = applyActions(handle);
        if (err != 0)
        {
          printf("Error: Failed to apply actions to %" PRId64 " with code %d\n", (int64_t)handle, err);
          return 183;
        }
      }
      return 0;
    }
    printf("[");
    for (i = 0; i < filter_handle_list_size; i++)
    {
      handle = (HWND)filter_handle_list[i];
      makeWindowJson(buffer, BUFFER_SIZE, handle);
      printf("%s%s", buffer, i + 1 < filter_handle_list_size ? ", " : "]");
    }
    return 0;
  }

  if (is_filter_foreground)
  {
    handle = GetForegroundWindow();
    is_win = handle != NULL && IsWindow(handle);
    if (verbose)
      printf("[Verbose] Single handle source: \"filter_foreground\": %" PRId64 " (%d)\n", (int64_t)handle, (int)is_win);
    if (!is_win)
    {
      printf("Error: Could not find foreground window\n");
      return 207;
    }
    if (isActionMode)
    {
      err = applyActions(handle);
      if (err != 0)
      {
        printf("Error: Failed to apply actions to %" PRId64 " with code %d\n", (int64_t)handle, err);
        return 216;
      }
      return 0;
    }
    makeWindowJson(buffer, BUFFER_SIZE, handle);
    printf("[%s]", buffer);
    return 0;
  }

  if (is_filter_parent == 0 && is_filter_desktop == 0 && isActionMode)
  {
    printf("Error: Cannot apply actions because there are no window filters\n");
    return 225;
  }
  if (is_filter_parent != 0)
  {
    if (verbose)
      printf("[Verbose] Starting handle source: \"filter_parent\"\n");
    handle = FindWindowExW((HWND)filter_parent, NULL, NULL, NULL);
    is_win = handle != NULL && IsWindow(handle);
    if (!is_win)
    {
      printf("Error: Could not find parent window\n");
      return 237;
    }
  }
  if (is_filter_desktop != 0)
  {
    if (verbose)
      printf("[Verbose] Starting handle source: \"filter_desktop\"\n");
    handle = FindWindowExW(NULL, NULL, NULL, NULL);
    is_win = handle != NULL && IsWindow(handle);
    if (!is_win)
    {
      printf("Error: Could not find desktop window\n");
      return 249;
    }
  }
  if (handle == NULL)
  {
    printf("Error: Could not find starting window\n");
    return 258;
  }
  is_win = IsWindow(handle);
  if (!is_win)
  {
    printf("Error: Starting window handle is not valid\n");
    return 266;
  }
  i = 0;
  while (handle && is_win)
  {
    if (isActionMode)
    {
      err = applyActions(handle);
      if (err != 0)
      {
        printf("Error: Failed to apply actions to %" PRId64 " with code %d\n", (int64_t)handle, err);
        return 272;
      }
      last_next = GetWindow(handle, GW_HWNDNEXT);
    }
    else
    {
      makeWindowJson(buffer, BUFFER_SIZE, handle);
      printf("%s%s", i == 0 ? "[" : ", ", buffer);
    }
    handle = last_next;
    is_win = handle != NULL && handle != 0 && IsWindow(handle);
    i++;
  }
  if (!isActionMode)
  {
    printf("]");
  }
  return 0;
}

int isMatchingString(const char *str1, const char *str2)
{
  size_t i;
  if (str1 == NULL || str2 == NULL || SMALL_BUFFER_SIZE == 0)
  {
    return 0;
  }
  for (i = 0; i < SMALL_BUFFER_SIZE; i++)
  {
    if (str1[i] == '\0' && str2[i] == '\0')
    {
      if (verbose)
        printf("[Verbose] Matched string of size %llu: \"%s\"\n", i, str1);
      return 1;
    }
    if (str1[i] == '\0' || str2[i] == '\0')
    {
      // if (verbose && i != 0) printf("[Verbose] Unmatching at %llu: \"%s\" != \"%s\"\n", i, str1, str2);
      return 0;
    }
    if (str1[i] == str2[i])
    {
      continue;
    }
    if (str1[i] >= 'A' && str1[i] <= 'Z' && str1[i] + 32 == str2[i])
    {
      continue;
    }
    if (str1[i] >= 'a' && str1[i] <= 'z' && str1[i] - 32 == str2[i])
    {
      continue;
    }
    // if (verbose && i != 0) printf("[Verbose] Unmatching at %llu: \"%s\" != \"%s\"\n", i, str1, str2);
    return 0;
  }
  return 0;
}
char parse_buffer[32];
int safeParseLong(long *target)
{
  if (parse_buffer[0] == '\0' || (parse_buffer[0] < '0' && parse_buffer[0] > '9' && parse_buffer[0] != '-'))
  {
    return 0;
  }
  if (parse_buffer[0] == '0' && parse_buffer[1] == '\0')
  {
    *target = 0;
    return 1;
  }
  int base = 10;
  int j;
  for (j = 0; j < 32; j++)
  {
    if (parse_buffer[j] == '\0')
    {
      break;
    }
    if (j == 0 && parse_buffer[j] == '-')
    {
      continue;
    }
    if (parse_buffer[j] == 'x' && ((j == 1 && parse_buffer[0] == '0') || (j == 2 && parse_buffer[0] == '-' && parse_buffer[1] == '0')))
    {
      base = 16;
      continue;
    }
    if (base == 16 && ((parse_buffer[j] >= 'A' && parse_buffer[j] <= 'F') || (parse_buffer[j] >= 'a' && parse_buffer[j] <= 'f')))
    {
      continue;
    }
    if (parse_buffer[j] >= '0' && parse_buffer[j] <= '9')
    {
      continue;
    }
    break;
  }
  if (j == 0)
    return 0;
  char *end = parse_buffer;
  long value = strtol(parse_buffer, &end, base);
  // if (verbose) printf("Interpreted: %lu from \"%s\"\n", value, parse_buffer);
  *target = value;
  return 1;
}

int main(int argn, char **argv)
{
  if (verbose)
    printf("[Verbose] Program started with %d arguments\n", argn);

  if (argn <= 1 || argv == NULL || argv[0] == NULL || argv[1] == NULL || argv[1][0] == '\0')
  {
    printHelp();
    return 1;
  }
  int isHelpArg;
  int isMessageArg;
  int isForegroundArg;
  int isDesktopArg;
  int isSetForegroundArg;
  int isSetTopArg;
  int isSetTopMostArg;
  int isShowArg;
  int isHideArg;
  int isMaximizeArg;
  int isMinimizeArg;
  int isTitleArg;
  int isHandleArg;
  int isParentArg;
  int isPidArg;
  int isStyleArg;
  int isExstyleArg;
  int isMoveArg;
  int isSizeArg;
  int isLongArg;
  int isWordArg;
  long v;
  int i;
  int j;
  char *str = NULL;
  char *flag = NULL;
  char *next = NULL;
  char c = '\0';
  for (i = 1; i < argn && argv[i] != NULL && argv[i][0] != '\0'; i++)
  {
    str = argv[i];
    c = argv[i][0];
    // Special starting "*" argument
    if (c == '*' && i == 1 && argv[i][1] == '\0')
    {
      is_filter_desktop = 1;
      continue;
    }
    // Special starting numeric handle argument
    if (c >= '0' && c <= '9' && i == 1)
    {
      if (verbose)
        printf("[Verbose] First argument is numeric and will be interpreted as handle: \"%s\"\n", str);
      v = 0;
      for (v = 0; v < 31 && str[v] != '\0'; v++)
      {
        parse_buffer[v] = str[v];
      }
      parse_buffer[v] = '\0';
      v = 0;
      if (i + 1 > argn || !safeParseLong(&v))
      {
        printf("Error: Invalid starting numeric argument: \"%s\" from index %d\n", str, i);
        return 1;
      }
      is_filter_handle = 1;
      filter_handle_list[0] = v;
      filter_handle_list_size = 1;
      continue;
    }
    if (!(c == '-' || c == '\\' || c == '=' || c == '//' || c == '*' || c == '+'))
    {
      printf("Error: Unexpected argument prefix \"%c\" at index %d (use \"--help\" for help)\n", c, i);
      return 1;
    }
    c = argv[i][1];
    if (c == 'h' || c == 'H')
    {
      printHelp();
      return argv[i][2] == '\0' ? 0 : 1;
    }
    if (!(c == '-' || c == '\\' || c == '=' || c == '//' || c == '*' || c == '+'))
    {
      printf("Error: Unexpected incomplete flag argument \"%s\" at index %d (use \"--help\" for help)\n", str, i);
      return 1;
    }
    flag = &str[2];
    c = flag[0];
    if (verbose)
      printf("[Verbose] Arg %d is a flag: \"%s\"\n", i, flag);
    isHelpArg = isMatchingString("help", flag);
    if (verbose)
      printf("[Verbose] isHelpArg: %d\n", isHelpArg);
    if (isHelpArg)
    {
      printHelp();
      return argn == 2 ? 0 : 1;
    }
    if (c == '-' || c == '\\' || c == '=' || c == '//' || c == '*' || c == '+' || c == '\0')
    {
      printf("Error: Unexpected long flag \"%s\" at index %d\n", str, i);
      return 1;
    }
    // Filters
    isMessageArg = isMatchingString("message", flag);
    isForegroundArg = isMatchingString("foreground", flag) || isMatchingString("f", flag) || isMatchingString("is-foreground", flag) || isMatchingString("top", flag) || isMatchingString("focus", flag) || isMatchingString("focused", flag) || isMatchingString("selected", flag) || isMatchingString("active", flag);
    isDesktopArg = isMatchingString("desktop", flag) || isMatchingString("dektop", flag) || isMatchingString("list", flag) || isMatchingString("root", flag) || isMatchingString("all", flag) || isMatchingString("screen", flag) || isMatchingString("display", flag) || isMatchingString("all", flag);

    if (isMessageArg != 0)
      is_filter_message = 1;
    if (isForegroundArg != 0)
      is_filter_foreground = 1;
    if (isDesktopArg != 0)
      is_filter_desktop = 1;

    if (isMessageArg || isForegroundArg || isDesktopArg)
    {
      if (verbose)
        printf("[Verbose] Interpreted standalone filtering argument at %d.\n", i);
      continue;
    }

    // Operations
    isSetForegroundArg = isMatchingString("set-foreground", flag) || isMatchingString("set-focus", flag) || isMatchingString("set-main", flag) || (is_filter_handle != 0 && isMatchingString("focus", flag));
    isSetTopArg = isMatchingString("set-top", flag) || isMatchingString("make-top", flag) || isMatchingString("raise", flag) || isMatchingString("bring-to-top", flag) || isMatchingString("bring-top", flag);
    isSetTopMostArg = isMatchingString("set-top-most", flag) || isMatchingString("make-top-most", flag) || isMatchingString("bring-to-top-most", flag) || isMatchingString("bring-top-most", flag);
    isShowArg = isMatchingString("show", flag) || isMatchingString("set-visible", flag);
    isHideArg = isMatchingString("hide", flag) || isMatchingString("set-invisible", flag);
    isMaximizeArg = isMatchingString("maximize", flag) || isMatchingString("set-maximize", flag) || isMatchingString("max", flag) || isMatchingString("undock", flag);
    isMinimizeArg = isMatchingString("minimize", flag) || isMatchingString("set-minimize", flag) || isMatchingString("min", flag) || isMatchingString("dock", flag);

    if (isSetForegroundArg != 0)
      is_action_set_foreground = 1;
    if (isSetTopArg != 0 && is_action_bring_to_top != 2)
      is_action_bring_to_top = 1;
    if (isSetTopMostArg != 0 && is_action_bring_to_top != 2)
      is_action_bring_to_top = 2;
    if (isShowArg != 0)
      is_action_show = 1;
    if (isHideArg != 0)
      is_action_hide = 1;
    if (isMinimizeArg != 0)
      is_action_minimize = 1;
    if (isMaximizeArg != 0)
      is_action_maximize = 1;

    if (isSetForegroundArg || isSetTopMostArg || isSetTopArg || isShowArg || isHideArg || isMaximizeArg || isMinimizeArg)
    {
      if (verbose)
        printf("[Verbose] Interpreted standalone operation argument at %d.\n", i);
      continue;
    }
    if (verbose)
      printf("[Verbose] Verifying possible standalone argument at %d.\n", i);
    if (i + 1 >= argn)
    {
      printf("Error: Invalid or incomplete argument after \"%s\" from index %d\n", argv[i], i);
      return 1;
    }
    next = argv[i + 1];
    isTitleArg = isMatchingString("title", flag);
    if (isTitleArg)
    {
      next = argv[i + 1];
      printf("isTitleArg src %s\n", next);
      is_filter_title = 1;
      for (filter_title_length = 0; filter_title_length < MIDDLE_BUFFER_SIZE; filter_title_length++)
      {
        c = next[filter_title_length];
        filter_title[filter_title_length] = c;
        if (filter_title[filter_title_length] == '\0')
        {
          break;
        }
      }
      filter_title[filter_title_length] = '\0';
      printf("titleArg size %zd\n", filter_title_length);
      printf("titleArg: %s\n", filter_title);
      i++;
      continue;
    }

    if (i + 1 < argn)
    {
      if (verbose)
        printf("[Verbose] Parsing numeric argument at %d.\n", i + 1);
      v = 0;
      for (v = 0; v < 31 && argv[i + 1][v] != '\0'; v++)
      {
        parse_buffer[v] = argv[i + 1][v];
      }
      parse_buffer[v] = '\0';
      v = 0;
      if (i + 1 > argn || !safeParseLong(&v))
      {
        printf("Error: Invalid argument after \"%s\" from index %d\n", argv[i], i);
        return 1;
      }
      if (verbose)
      {
        printf("[Verbose] Numeric argument at %d: \"%s\"\n", i, argv[i]);
      }
    }

    isHandleArg = isMatchingString("handle", flag);
    if (verbose)
      printf("[Verbose] isHandleArg %d\n", isHandleArg);
    isParentArg = isMatchingString("parent", flag);
    if (verbose)
      printf("[Verbose] isParentArg %d\n", isParentArg);
    isPidArg = isMatchingString("pid", flag) || isMatchingString("parent", flag);
    if (verbose)
      printf("[Verbose] isPidArg %d\n", isPidArg);
    isStyleArg = isMatchingString("style", flag);
    if (verbose)
      printf("[Verbose] isStyleArg %d\n", isStyleArg);
    isExstyleArg = isMatchingString("exstyle", flag);
    if (verbose)
      printf("[Verbose] isExstyleArg %d\n", isExstyleArg);
    isMoveArg = isMatchingString("move", flag) || isMatchingString("pos", flag);
    if (verbose)
      printf("[Verbose] isMoveArg %d\n", isMoveArg);
    isSizeArg = isMatchingString("size", flag) || isMatchingString("resize", flag);
    if (verbose)
      printf("[Verbose] isSizeArg %d\n", isSizeArg);
    i++;
    if (isHandleArg)
    {
      if (is_filter_handle != 1)
      {
        is_filter_handle = 1;
        filter_handle_list_size = 0;
      }
      if (verbose)
        printf("[Verbose] Setting handler filter [%d] to %" PRId64 "\n", (int)filter_handle_list_size, (int64_t)v);
      if (filter_handle_list_size >= SMALL_BUFFER_SIZE)
      {
        printf("Error: Too many window handles have been specified (Max is %d)\n", (int)SMALL_BUFFER_SIZE);
        return 1;
      }
      filter_handle_list[filter_handle_list_size] = v;
      filter_handle_list_size++;
      continue;
    }
    if (isParentArg)
    {
      if (is_filter_parent == 1)
      {
        printf("Error: Window parent has already been specified (duplicated at index %d)\n", i);
        return 1;
      }
      is_filter_parent = 1;
      filter_parent = v;
      continue;
    }
    if (isPidArg)
    {
      if (is_filter_pid == 1)
      {
        printf("Error: Window pid has already been specified (duplicated at index %d)\n", i);
        return 1;
      }
      is_filter_pid = 1;
      filter_pid = v;
      continue;
    }
    if (isStyleArg)
    {
      is_filter_style = 1;
      filter_style = v;
      continue;
    }
    if (isExstyleArg)
    {
      is_filter_exstyle = 1;
      filter_exstyle = v;
      continue;
    }
    if (isMoveArg)
    {
      is_action_move = 1;
      action_x = v;
    }
    if (isSizeArg)
    {
      is_action_size = 1;
      action_w = v;
    }
    v = 0;
    for (v = 0; v < 31 && argv[i + 1][v] != '\0'; v++)
    {
      parse_buffer[v] = argv[i + 1][v];
    }
    parse_buffer[v] = '\0';
    v = 0;
    if (i + 1 > argn || !safeParseLong(&v))
    {
      printf("Error: Invalid argument after \"%s\" from index %d\n", argv[i], i);
      return 1;
    }
    if (isMoveArg)
    {
      action_y = v;
      i++;
    }
    if (isSizeArg)
    {
      action_h = v;
      i++;
    }
  }

  if (verbose)
  {
    if (is_filter_foreground)
      printf("[Verbose] filter_foreground.\n");
    if (is_filter_desktop)
      printf("[Verbose] filter_desktop.\n");
    if (is_filter_message)
      printf("[Verbose] filter_message.\n");
    if (is_filter_title)
      printf("[Verbose] filter_title: [%zd] %s\n", (int64_t)filter_title_length, filter_title);
    if (is_filter_handle && filter_handle_list_size > 0)
      printf("[Verbose] filter_handle_list[0]: %" PRId64 "\n", (int64_t)filter_handle_list[0]);
    if (is_filter_handle && filter_handle_list_size > 1)
      printf("[Verbose] filter_handle_list[1]: %" PRId64 "\n", (int64_t)filter_handle_list[1]);
    if (is_filter_handle && filter_handle_list_size > 2)
      printf("[Verbose] filter_handle_list[2]: %" PRId64 "\n", (int64_t)filter_handle_list[2]);
    if (is_filter_parent)
      printf("[Verbose] filter_parent: %" PRId64 "\n", (int64_t)filter_parent);
    if (is_filter_pid)
      printf("[Verbose] filter_pid: %" PRId64 "\n", (int64_t)filter_pid);
    if (is_filter_style)
      printf("[Verbose] filter_style: %" PRId64 "\n", (int64_t)filter_style);
    if (is_filter_exstyle)
      printf("[Verbose] filter_exstyle: %" PRId64 "\n", (int64_t)filter_exstyle);
    if (is_action_set_foreground)
      printf("[Verbose] is_action_set_foreground.\n");
    if (is_action_bring_to_top)
      printf("[Verbose] is_action_bring_to_top: %d\n", is_action_bring_to_top);
    if (is_action_move)
      printf("[Verbose] is_action_move: %" PRId64 ", %" PRId64 "\n", (int64_t)action_x, (int64_t)action_y);
    if (is_action_size)
      printf("[Verbose] is_action_size: %" PRId64 ", %" PRId64 "\n", (int64_t)action_w, (int64_t)action_h);
    if (is_action_show)
      printf("[Verbose] is_action_show: %" PRId64 "\n", (int64_t)is_action_show);
    if (is_action_hide)
      printf("[Verbose] is_action_hide: %" PRId64 "\n", (int64_t)is_action_hide);
    if (is_action_maximize)
      printf("[Verbose] is_action_maximize: %" PRId64 "\n", (int64_t)is_action_maximize);
    if (is_action_minimize)
      printf("[Verbose] is_action_minimize: %" PRId64 "\n", (int64_t)is_action_minimize);
  }

  if (is_filter_message || is_filter_title || is_filter_pid || is_filter_style || is_filter_exstyle)
  {
    printf("Error: Selected filters are not implemented\n");
    return 1;
  }

  return startProgram();
}

size_t makeWindowJson(char *buffer, size_t buffer_size, HWND h)
{
  int is_win = h == NULL ? 0 : IsWindow(h);
  if (!is_win)
  {
    size_t i = 0;
    i += snprintf(&buffer[i], buffer_size - i, "{\"target\": %" PRId64 ", \"error\": \"Window not found\"}", (int64_t)h);
    return i;
  }
  title[0] = '\0';
  title[1] = '\0';
  title_length = is_win ? GetWindowText(h, title, MIDDLE_BUFFER_SIZE) : 0;
  module[0] = '\0';
  module[1] = '\0';
  module_length = is_win ? GetWindowModuleFileName(h, module, MIDDLE_BUFFER_SIZE) : 0;
  class[0] = '\0';
  class[1] = '\0';
  class_length = is_win ? GetClassName(h, class, MIDDLE_BUFFER_SIZE) : 0;
  HWND parent = is_win ? GetParent(h) : NULL;
  HWND next = is_win ? GetWindow(h, GW_HWNDNEXT) : NULL;
  last_next = next;
  HWND child = is_win ? GetWindow(h, GW_CHILD) : NULL;
  DWORD pid = 0;
  DWORD thread = GetWindowThreadProcessId(h, &pid);
  exec_file_path_length = 0;
  if (pid > 0)
  {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess != NULL)
    {
      exec_file_path_length = GetModuleFileNameEx(hProcess, NULL, exec_file_path, MIDDLE_BUFFER_SIZE);
      CloseHandle(hProcess);
    }
  }
  LONG style = GetWindowLong(h, GWL_STYLE);
  int64_t exstyle = (int64_t)GetWindowLong(h, GWL_EXSTYLE);
  int is_unicode = IsWindowUnicode(h);
  int is_visible = IsWindowVisible(h);
  int is_popup = (style & WS_POPUP) > 0;
  int is_contained = (style & WS_CLIPSIBLINGS) > 0;
  int is_bordered = ((style & WS_BORDER) > 0) || ((style & WS_THICKFRAME) > 0);
  int is_scrollable = (style & WS_HSCROLL) > 0 || (style & WS_VSCROLL) > 0;
  int is_visible_alt = (style & WS_VISIBLE) > 0;
  int is_minimized = (style & WS_MINIMIZE) > 0;
  int is_topmost = (exstyle & WS_EX_TOPMOST) > 0;
  int is_transparent = (exstyle & WS_EX_TRANSPARENT) > 0;

  rect.top = 0;
  rect.left = 0;
  rect.right = 0;
  rect.bottom = 0;
  if (0 == GetWindowRect(h, &rect))
  {
    rect.top = 0;
    rect.left = 0;
    rect.right = 0;
    rect.bottom = 0;
  }
  return putWindowJson(
      buffer,
      BUFFER_SIZE,
      h,
      title,
      title_length,
      module,
      module_length,
      (char *)exec_file_path,
      exec_file_path_length,
      class,
      class_length,
      parent,
      next,
      child,
      pid,
      thread,
      style,
      exstyle,
      is_unicode,
      is_visible,
      is_popup,
      is_contained,
      is_bordered,
      is_scrollable,
      is_visible_alt,
      is_minimized,
      is_topmost,
      is_transparent,
      &rect);
}

char escape[BUFFER_SIZE];
size_t escape_length = 0;

int escapeJsonString(char *text, size_t *text_length)
{
  int updated = 0;
  size_t i = 0;
  size_t i_max = *text_length;
  size_t j = 0;
  size_t j_max = BUFFER_SIZE - 1;
  for (i = 0; i < i_max; i++)
  {
    char c = text[i];
    if (c == '\\')
    {
      updated = 1;
      escape[j++] = '/';
    }
    else if ((c == '"' || c == '\\' || c == '\n' || c == '\t') && j + 2 < j_max)
    {
      updated = 1;
      escape[j++] = '\\';
      escape[j++] = c == '\n' ? 'n' : (c == '\t' ? 't' : c);
    }
    else if (j < j_max)
    {
      escape[j++] = c;
    }
  }
  if (updated == 0)
    return 0;
  for (i = 0; i <= j; i++)
  {
    text[i] = i == j ? '\0' : escape[i];
  }
  *text_length = i;
  return updated ? 1 : 0;
}

size_t putWindowJson(
    char *buffer,
    size_t buffer_size,
    HWND h,
    char *title,
    size_t title_size,
    char *module,
    size_t module_size,
    char *exec,
    size_t exec_length,
    char *class,
    size_t class_size,
    HWND parent,
    HWND next,
    HWND child,
    DWORD pid,
    DWORD thread,
    LONG style,
    int64_t exstyle,
    int is_unicode,
    int is_visible,
    int is_popup,
    int is_contained,
    int is_bordered,
    int is_scrollable,
    int is_visible_alt,
    int is_minimized,
    int is_topmost,
    int is_transparent,
    RECT *rect)
{
  size_t i = 0;
  i += snprintf(&buffer[i], buffer_size - i, "{\"handle\": %" PRId64 ", ", (int64_t)h);
  if (title_length > 0 && title != NULL)
  {
    escapeJsonString(title, &title_length);
    i += snprintf(&buffer[i], buffer_size - i, "\"title\": \"%s\", ", (char *)title);
  }
  if (module_length > 0 && module != NULL)
  {
    escapeJsonString(module, &module_length);
    i += snprintf(&buffer[i], buffer_size - i, "\"module\": \"%s\", ", (char *)module);
  }
  if (exec_length > 0 && exec != NULL)
  {
    escapeJsonString(exec, &exec_length);
    i += snprintf(&buffer[i], buffer_size - i, "\"executable\": \"%s\", ", exec);
  }
  if (class_length > 0 && class != NULL)
  {
    escapeJsonString(class, &class_length);
    i += snprintf(&buffer[i], buffer_size - i, "\"classname\": \"%s\", ", (char *)class);
  }
  i += snprintf(&buffer[i], buffer_size - i, "\"parent\": %" PRId64 ", ", (int64_t)parent);
  if (next != 0)
  {
    i += snprintf(&buffer[i], buffer_size - i, "\"sibling\": %" PRId64 ", ", (int64_t)next);
  }
  if (child != NULL)
    i += snprintf(&buffer[i], buffer_size - i, "\"child\": %" PRId64 ", ", (int64_t)child);
  i += snprintf(&buffer[i], buffer_size - i, "\"pid\": %" PRId64 ", ", (int64_t)pid);

  i += snprintf(&buffer[i], buffer_size - i, "\"thread\": %" PRId64 ", ", (int64_t)thread);
  i += snprintf(&buffer[i], buffer_size - i, "\"style\": %" PRId64 ", ", (int64_t)style);
  i += snprintf(&buffer[i], buffer_size - i, "\"exstyle\": %" PRId64 ", ", (int64_t)exstyle);
  i += snprintf(&buffer[i], buffer_size - i, "\"visible\": %s, ", is_visible ? "true" : "false");
  i += snprintf(&buffer[i], buffer_size - i, "\"unicode\": %s", is_unicode ? "true" : "false");
  if (is_popup)
    i += snprintf(&buffer[i], buffer_size - i, ", \"popup\": true");
  if (is_contained)
    i += snprintf(&buffer[i], buffer_size - i, ", \"contained\": true");
  if (is_bordered)
    i += snprintf(&buffer[i], buffer_size - i, ", \"bordered\": true");
  if (is_scrollable)
    i += snprintf(&buffer[i], buffer_size - i, ", \"scrollable\": true");
  if (is_visible_alt != is_visible)
    i += snprintf(&buffer[i], buffer_size - i, ", \"visible_alt\": true");
  if (is_minimized)
    i += snprintf(&buffer[i], buffer_size - i, ", \"minimized\": true");
  if (is_topmost)
    i += snprintf(&buffer[i], buffer_size - i, ", \"topmost\": true");
  if (is_transparent)
    i += snprintf(&buffer[i], buffer_size - i, ", \"transparent\": true");
  int64_t top = (int64_t)(rect != NULL ? rect->top : 0);
  int64_t right = (int64_t)(rect != NULL ? rect->right : 0);
  int64_t bottom = (int64_t)(rect != NULL ? rect->bottom : 0);
  int64_t left = (int64_t)(rect != NULL ? rect->left : 0);
  if (top != 0 || left != 0 || right != 0 || bottom != 0)
    i += snprintf(&buffer[i], buffer_size - i, ", \"top\": %" PRId64 ", \"right\": %" PRId64 ", \"bottom\": %" PRId64 ", \"left\": %" PRId64 "", top, right, bottom, left);
  i += snprintf(&buffer[i], buffer_size - i, "}");
  buffer[i] = '\0';
  return i;
}