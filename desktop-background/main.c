#include "windows.h"
#include "stdio.h"
#include <psapi.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Advapi32.lib")

#define verbose 0

void printHelp()
{
  printf("desktop-background - Utility to interact with the desktop background image\n");
  printf("\n");
  printf("Usage:\n");
  printf("\t desktop-background --get         Retrieve the current desktop background image information.\n");
  printf("\t desktop-background --set <path>  Update the current desktop image to the specified image.\n");
  printf("\t desktop-background --help        Display this help text.\n");
  printf("\n");
}

#define BUFFER_SIZE 2048
#define MIDDLE_BUFFER_SIZE MAX_PATH * 2

HKEY key_handle;
DWORD key_type;
DWORD key_buffer_size = BUFFER_SIZE;
char key_value[BUFFER_SIZE];
char global_out_buffer[BUFFER_SIZE];
size_t global_out_buffer_size = (size_t)(BUFFER_SIZE / 2);
size_t global_out_buffer_written = 0;

size_t putJsonStringEscaped(char *input, size_t input_length, char *output, size_t *output_length)
{
  size_t i = 0;
  size_t j = 0;
  char c;

  if (input_length <= 0) {
    if (verbose > 1)
      printf("[Verbose] Escaping input length is zero.\n");
    return 0;
  }

  if (output_length <= 0) {
    if (verbose > 1)
      printf("[Verbose] Escaping output length is zero.\n");
    return 0;
  }
  
  if (verbose > 1)
    printf("[Verbose] Starting escaped json string method.\n");

  for (i = 0; j < output_length && i < input_length; i++) {
    if (verbose > 1)
      printf("[Verbose] Escaping starting at #%zd\n", i);
    c = input[i];
    if (verbose > 1)
      printf("[Verbose] Escaping at #%zd has char \"%d\"\n", i, (int) c);
    if (c == '\0') {
      break;
    }
    if (c != '\\' && c != '"' && c != '\n' && c != '\t') {
      output[j] = c;
      j++;
      continue;
    }

    if (verbose > 1)
      printf("[Verbose] Found special character at #%zd: %c\n", i, c);

    output[j] = '\\';
    j++;

    if (c == '\n' || c == '\t')
      c = c == '\n' ? 'n' : 't';

    output[j] = c;
    j++;
  }

  if (j >= output_length)
    j = output_length - 1;

  output[j] = '\0';

  if (verbose > 1)
    printf("[Verbose] Completed escaping process.\n");

  return j;
}

size_t putJsonObjectSingleKey(char *out_buffer, size_t out_buffer_size, char *key, char *value)
{
  size_t i = 0;
  i += (size_t)snprintf(&out_buffer[i], out_buffer_size - i, "{\"");
  i += putJsonStringEscaped(key, (size_t)BUFFER_SIZE/2, &out_buffer[i], out_buffer_size - i);
  i += (size_t)snprintf(&out_buffer[i], out_buffer_size - i, "\": \"");
  i += putJsonStringEscaped(value, (size_t)BUFFER_SIZE/2, &out_buffer[i], out_buffer_size - i);
  i += (size_t)snprintf(&out_buffer[i], out_buffer_size - i, "\"}");
  out_buffer[i] = '\0';
  return i;
}

int handleGetRequest()
{
  if (verbose)
    printf("[Verbose] Handling get request by loading registry value for reading.\n");
  Sleep(100);
  if (verbose)
    printf("[Verbose] Registry key: \"Control Panel\\Desktop\".\n");
  if (RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\Desktop", 0, KEY_READ, &key_handle) != ERROR_SUCCESS)
  {
    if (verbose)
      printf("Error opening registry key.\n");
    global_out_buffer_written = putJsonObjectSingleKey(global_out_buffer, global_out_buffer_size, "error", "Could not open registry for reading");
    return 1;
  }

  if (verbose)
    printf("[Verbose] Opened desktop registry value successfully.\n");

  if (RegQueryValueEx(key_handle, "Wallpaper", NULL, &key_type, (LPBYTE)key_value, &key_buffer_size) != ERROR_SUCCESS)
  {
    if (verbose)
      printf("[Verbose] Could not read desktop registry value successfully.\n");
    global_out_buffer_written = putJsonObjectSingleKey(global_out_buffer, global_out_buffer_size, "error", "Could not read registry value after opening");
    RegCloseKey(key_handle);
    return 1;
  }
  if (verbose)
    printf("[Verbose] Read desktop registry value successfully with size %d.\n", key_buffer_size);

  if (key_type == REG_SZ && key_value != NULL && key_value[0] == '\0')
  {
    if (verbose)
      printf("[Verbose] The wallpaper file path is empty.\n");
  }
  else if (key_type == REG_SZ)
  {
    if (verbose)
      printf("[Verbose] The wallpaper registry is a REG_SZ: \"%s\"\n", key_value);
  }
  else
  {
    if (verbose)
      printf("[Verbose] The wallpaper registry type is: %" PRId64 " (0x%" PRIX64 ")\n", (uint64_t)key_type, (uint64_t)key_type);
    if (verbose)
      printf("[Verbose] The wallpaper value is: \"%s\"\n", key_value);
  }
  global_out_buffer_written = putJsonObjectSingleKey(global_out_buffer, global_out_buffer_size, "path", key_value);
  RegCloseKey(key_handle);
  return 0;
}

int handleSetRequest(const char *path)
{
  if (verbose)
    printf("[Verbose] Handling set request by opening registry value for writing\n");
  Sleep(100);

  if (verbose)
    printf("[Verbose] Registry key: \"Control Panel\\Desktop\"\n");

  if (RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\Desktop", 0, KEY_WRITE, &key_handle) != ERROR_SUCCESS)
  {
    if (verbose)
      printf("[Verbose] Error opening registry key for writing.\n");
    global_out_buffer_written = putJsonObjectSingleKey(global_out_buffer, global_out_buffer_size, "error", "Could not open registry to write");
    return 1;
  }
  if (verbose)
    printf("[Verbose] Opened desktop registry value successfully.\n");

  BYTE normalized[BUFFER_SIZE];
  size_t i = 0;
  size_t j = 0;
  for (i = 0; i < BUFFER_SIZE && j+1 < BUFFER_SIZE; i++)
  {
    if (path[i] == '"')
    {
      continue;
    }
    if (path[i] != '\\' && path[i] != '/')
    {
      normalized[j] = path[i];
      j++;
      continue;
    }
    if (j > 0 && (normalized[j - 1] == '\\' || normalized[j - 1] == '/'))
    {
      continue;
    }
    normalized[j] = '\\';
    j++;
  }
  normalized[j] = '\0';
  if (verbose)
    printf("[Verbose] The normalized buffer from path has %d bytes.\n", (int)j);

  if (RegSetValueEx(key_handle, "Wallpaper", 0, REG_SZ, (const BYTE *)normalized, strlen(normalized) + 1) != ERROR_SUCCESS)
  {
    if (verbose)
      printf("[Verbose] Error setting registry value.\n");
    global_out_buffer_written = putJsonObjectSingleKey(global_out_buffer, global_out_buffer_size, "error", "Could not set registry value");
    RegCloseKey(key_handle);
    return 1;
  }

  SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void *)normalized, SPIF_SENDCHANGE);
  if (verbose)
    printf("[Verbose] Desktop background set to: %s\n", normalized);
  RegCloseKey(key_handle);
  global_out_buffer_written = putJsonObjectSingleKey(global_out_buffer, global_out_buffer_size, "path", normalized);
  return 0;
}

int main(int argn, char **argv)
{
  int isGetArg = 0;
  int isSetArg = 0;
  int r = 0;

  if (verbose)
    printf("[Verbose] Program started with %d arguments\n", argn);

  if (argn <= 1 || argv == NULL || argv[0] == NULL || argv[1] == NULL || argv[1][0] == '\0' || argv[1][0] != '-')
  {
    printf("Invalid program arguments\n");
    printHelp();
    return 1;
  }

  if ((argv[1][1] == 'h' || argv[1][1] == 'H' || argv[1][1] == 'v' || argv[1][1] == '?') && argv[1][2] == '\0')
  {
    printHelp();
    return 0;
  }

  if (argv[1][1] == '-' && (argv[1][2] == 'h' || argv[1][2] == 'H'))
  {
    printHelp();
    return 0;
  }

  if (argv[1][1] == 'g' || argv[1][1] == 'G' || argv[1][1] == 'r' || argv[1][1] == 'R')
  {
    if (verbose)
      printf("[Verbose] Program mode: Get (small alias)\n");
    isGetArg = 1;
  }
  else if (argv[1][1] == '-' && (argv[1][2] == 'g' || argv[1][2] == 'G' || argv[1][2] == 'r' || argv[1][2] == 'R'))
  {
    if (verbose)
      printf("[Verbose] Program mode: Get\n");
    isGetArg = 1;
  }
  if (argv[1][1] == 's' || argv[1][1] == 'S' || argv[1][1] == 'w' || argv[1][1] == 'W')
  {
    if (verbose)
      printf("[Verbose] Program mode: Set (small alias)\n");
    isSetArg = 1;
  }
  else if (argv[1][1] == '-' && (argv[1][2] == 's' || argv[1][2] == 'S' || argv[1][2] == 'w' || argv[1][2] == 'W'))
  {
    if (verbose)
      printf("[Verbose] Program mode: Set\n");
    isSetArg = 1;
  }
  if (isGetArg == 1 && isSetArg == 0)
  {
    if (verbose)
      printf("[Verbose] Starting get request from main.\n");
    r = handleGetRequest();
    if (verbose)
      printf("[Verbose] Handled request return code: %d.\n", r);
    if (verbose)
      printf("[Verbose] Output written size: %d.\n", global_out_buffer_written);
    if (global_out_buffer_written != 0)
    {
      puts(global_out_buffer);
    }
    else
    {
      printf("{\"error\": \"Program output is empty\"}");
      if (r == 0)
        r = 1;
    }
    return r;
  }
  if (isGetArg == 0 && isSetArg == 1)
  {
    if (argn == 3 && argv[2] != NULL)
    {
      if (verbose)
        printf("[Verbose] Starting set request from main to \"%s\".\n", argv[2]);
      r = handleSetRequest(argv[2]);
      if (verbose)
        printf("[Verbose] Handled request return code: %d.\n", r);
      if (verbose)
        printf("[Verbose] Output written size: %d.\n", global_out_buffer_written);
      if (global_out_buffer_written != 0)
      {
        puts(global_out_buffer);
      }
      else
      {
        printf("{\"error\": \"Program output is empty\"}");
        if (r == 0)
          r = 1;
      }
      return r;
    }
    if (verbose)
      printf("[Verbose] Could not handle set request because of invalid arguments\n");
  }
  if (verbose)
    printf("[Verbose] Displaying help because of unhandled program arguments\n");
  printHelp();
  return 1;
}