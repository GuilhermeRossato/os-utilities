#include "windows.h"
#include "stdio.h"

#pragma comment (lib, "User32.lib")

int verbose = 0;

#define TBUFFER_SIZE 1024 * 1024
#define MBUFFER_SIZE 256

char text[TBUFFER_SIZE];
size_t text_length = 0;
char mode_arg[MBUFFER_SIZE];
size_t mode_index = 0;

void printHelp()
{
  printf("clipboard-text - Utility to read and write clipboard text data\n");
  printf("\n");
  printf("Usage:\n");
  printf("\tclipboard-text --read             Print the clipboard data stored in plain text format.\n");
  printf("\tclipboard-text --write <text...>  Set the clipboard data from the text of the imediate program arguments.\n");
  printf("\tclipboard-text --file <path>      Loads the text content of a file and write it to the clipboard data.\n");
  printf("\tclipboard-text --help             Print usage instructions and arguments.\n");
}

long setClipboardDataFormat(UINT format, char *src_buffer, size_t src_size)
{
  if (format == 0 || src_buffer == NULL || src_size <= 0)
  {
    printf("Error: Invalid or insufficient arguments to set clipboard data\n");
    return -11;
  }
  else if (verbose)
    printf("[Verbose] Allocating global memory of size %llu\n", src_size);

  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, src_size);

  if (handle == NULL)
  {
    if (verbose)
      printf("[Verbose] GlobalAlloc returned NULL handle using GMEM_MOVABLE flag\n");

    printf("Error: GlobalAlloc failed to allocate %llu bytes\n", src_size);
    return -12;
  }

  char *dst_buffer = GlobalLock(handle);
  if (dst_buffer == NULL)
  {
    if (verbose)
      printf("[Verbose] GlobalLock returned NULL pointer\n");

    printf("Error: GlobalLock failed to return clipboard buffer\n");
    return -12;
  }

  dst_buffer[0] = '\0';
  dst_buffer[src_size - 1] = '\0';

  memcpy(dst_buffer, src_buffer, src_size);

  if (verbose)
    printf("[Verbose] Copy of %llu bytes to target clipboard buffer complete\n", src_size);

  GlobalUnlock(handle);

  if (0 == OpenClipboard(0))
  {
    printf("Error: OpenClipboard failed\n");
    return -13;
  }
  else if (verbose)
    printf("[Verbose] OpenClipboard success\n");

  int b = EmptyClipboard();
  if (verbose)
    printf("[Verbose] EmptyClipboard returned %d\n", b);

  HANDLE c = SetClipboardData(format, handle);
  if (verbose)
    printf("[Verbose] SetClipboardData returned %s handle\n", c == NULL ? "null" : "non-null");

  int d = CloseClipboard();
  if (verbose)
    printf("[Verbose] CloseClipboard returned %d\n", d);

  return src_size;
}
long putClipboardFormatData(UINT format, char *dst_buffer, size_t dst_limit)
{
  if (format == 0 || dst_buffer == NULL || dst_limit <= 3)
  {
    printf("Error: Invalid format or insufficient arguments\n");
    return -1;
  }

  dst_buffer[0] = '\0';
  dst_buffer[1] = '\0';

  if (0 == OpenClipboard(0))
  {
    printf("Error: OpenClipboard failed\n");
    return -2;
  }
  else if (verbose)
    printf("[Verbose] OpenClipboard was successful\n");

  HANDLE handle = GetClipboardData(format);

  if (handle == NULL)
  {
    if (verbose)
      printf("[Verbose] GetClipboardData returned NULL and CloseClipboard returned %d\n", CloseClipboard());
    else
      CloseClipboard();
    printf("Error: GetClipboardData failed to retrieve handle for clipboard format of code %d\n", format);
    return -3;
  }
  else if (verbose)
    printf("[Verbose] GetClipboardData was successful\n");

  SIZE_T src_size = GlobalSize(handle);

  if (verbose)
    printf("[Verbose] GlobalSize returned %llu\n", src_size);

  char *src_buffer = GlobalLock(handle);

  if (src_buffer == NULL)
  {
    if (verbose)
      printf("[Verbose] GlobalLock returned NULL and CloseClipboard returned %d\n", CloseClipboard());
    else
      CloseClipboard();
    printf("Error: GlobalLock failed for clipboard format of code %d\n", format);
    return -4;
  }
  else if (verbose)
    printf("[Verbose] GlobalLock was successful\n");

  size_t copy_size = src_size < dst_limit ? src_size : dst_limit;

  if (verbose)
    printf("[Verbose] Copying %llu bytes to target buffer\n", copy_size);

  dst_buffer[0] = '\0';
  dst_buffer[1] = '\0';
  dst_buffer[copy_size - 2] = '\0';
  dst_buffer[copy_size - 1] = '\0';
  memcpy(dst_buffer, src_buffer, copy_size);

  if (0 == GlobalUnlock(handle))
  {
    printf("Error: GlobalUnlock failed\n");
    return -5;
  }

  if (verbose)
    printf("[Verbose] GlobalUnlock was successful and CloseClipboard returned %d\n", CloseClipboard());
  else
    CloseClipboard();

  if (copy_size > 0) {
    if (dst_buffer[copy_size - 1] != '\0') {
      printf("Error: Unexpected sufix character on target buffer after read\n");
    }
    copy_size--;
  }
  return (long)copy_size;
}

int isMatchingString(char *str1, char *str2, size_t max_size)
{
  size_t i;
  if (str1 == NULL || str2 == NULL)
  {
    return 0;
  }
  for (i = 0; i < max_size; i++)
  {
    if (str1[i] == '\0' && str2[i] == '\0')
    {
      if (verbose)
        printf("[Verbose] Matched string of size %llu: \"%s\"\n", i, str1);
      return 1;
    }
    if (str1[i] == '\0' || str2[i] == '\0')
    {
      return 0;
    }
    if (str1[i] >= 'A' && str1[i] <= 'Z' && str1[i] + 32 == str2[i])
    {
      continue;
    }
    if (str1[i] >= 'a' && str1[i] <= 'z' && str1[i] - 32 == str2[i])
    {
      continue;
    }
    if (str1[i] == str2[i])
    {
      continue;
    }
    return 0;
  }
  return i >= max_size ? 0 : 1;
}

int executeWriteFromFileMode(char *filePath)
{
  text[0] = '\0';
  text_length = 0;
  
  FILE *fp = fopen(filePath, "r");

  if (fp == NULL)
  {
    printf("Error: Failed to open specified file \"%s\" for reading\n", filePath);
    return 10;
  }

  text_length = fread(text, sizeof(char), TBUFFER_SIZE, fp);
  if (verbose)
    printf("[Verbose] Read %llu bytes from file\n", text_length);

  if (ferror(fp))
  {
    fclose(fp);
    printf("Error: Failed to read specified file \"%s\" to update clipboard data\n", filePath);
    return 12;
  }

  fclose(fp);

  if (verbose)
    printf("[Verbose] Last text buffer character code is %d at %lld\n", (int)((char)(text[text_length])), text_length);
  long status = setClipboardDataFormat(CF_TEXT, text, text_length);
  if (status == text_length) {
    return 0;
  }
  printf("Error: Update clipboard function returned %ld (Text size is %lld)\n", status, text_length);
  return 1;
}

int executeWriteFromArgsMode(int argn, const char **argv)
{
  text[0] = '\0';
  text_length = 0;

  size_t argv_index = 1;
  size_t argv_len;

  while (1)
  {
    argv_index++;
    if (argv_index >= argn || argv[argv_index] == NULL)
    {
      break;
    }
    argv_len = strnlen(argv[argv_index], TBUFFER_SIZE - 1) + 1;
    if (text_length + argv_len + 1 >= TBUFFER_SIZE - 1)
    {
      break;
    }
    if (text_length > 0 && text[text_length - 1] != ' ')
    {
      printf("Error: Text content is unexpectedly missing previous space character\n");
      return 11;
    }
    memcpy(&text[text_length], argv[argv_index], argv_len);
    text_length += argv_len;
    if (text_length + 1 >= TBUFFER_SIZE || text[text_length - 1] != '\0')
    {
      printf("Error: Unexpected buffer text content\n");
      return 12;
    }
    if (argv_index < argn)
    {
      text[text_length - 1] = ' ';
      continue;
    }
    break;
  }
  if (verbose)
    printf("[Verbose] Last text buffer character code is %d at %lld\n", (int)((char)(text[text_length])), text_length);
  long status = setClipboardDataFormat(CF_TEXT, text, text_length);
  if (status == text_length) {
    return 0;
  }
  printf("Error: Update clipboard function returned %ld (Text size is %lld)\n", status, text_length);
  return 1;
}

int executeReadMode()
{
  text[0] = '\0';
  text_length = 0;

  long status = putClipboardFormatData(CF_TEXT, text, TBUFFER_SIZE - 1);

  if (verbose)
    printf("[Verbose] Clipboard read data returned %lu\n", status);
  
  if (status < 0) {
    printf("Error: Failed to retrieve clipboard data\n");
    return 22;
  }

  text_length = status;

  if (verbose)
    printf("[Verbose] Last text buffer character code is %d\n", (int)((char)(text[text_length])));

  fwrite(text, sizeof(char), text_length, stdout);
  return 0;
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

  if (verbose)
    printf("[Verbose] The first program argument is \"%s\"\n", argv[1]);

  size_t part_index = 0;
  char c = argv[1][part_index];

  if (!(c == '-' || c == '\\' || c == '=' || c == '//' || c == '*' || c == '+'))
  {
    printf("Error: Invalid argument: expected either \"--read\", \"--write\", or \"--file\" (use \"--help\" for help)\n");
    return 1;
  }

  while (1)
  {
    c = argv[1][part_index];
    part_index++;
    if (c == '-' || c == '\\' || c == '=' || c == '//' || c == '*' || c == '+')
    {
      c = argv[1][part_index];
      part_index++;
    }
    if (c == '-' || c == '\\' || c == '=' || c == '//' || c == '*' || c == '+')
    {
      c = argv[1][part_index];
      part_index++;
    }
    if (mode_index + 1 >= MBUFFER_SIZE - 1)
      c = '\0';
    mode_arg[mode_index] = c;
    mode_index++;
    if (c == '\0')
      break;
  }

  if (mode_index == 0 || mode_index > MBUFFER_SIZE || mode_arg[mode_index] != '\0') {
    if (verbose)
      printf("[Verbose] The filtered argument at [%llu] has char code %d\n", mode_index, (int)((char)mode_arg[mode_index]));
    
    printf("Error: Invalid first argument format with size %lld\n", mode_index);
    return 1;
  }

  if (verbose)
    printf("[Verbose] The mode buffer has %lld bytes: \"%s\"\n", mode_index, mode_arg);

  int is_small_help_mode = (mode_arg[0] == 'h' || mode_arg[0] == 'v' || mode_arg[0] == 'H' || mode_arg[0] == 'V') && (mode_arg[1] == '\0');
  if (is_small_help_mode)
  {
    printHelp();
    return 0;
  }

  int is_help_mode = (isMatchingString("he", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("hel", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("hep", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("help", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("hlp", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("vv", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("vvv", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("ve", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("ver", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("version", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("info", mode_arg, MBUFFER_SIZE));

  if (is_help_mode)
  {
    printHelp();
    return 0;
  }

  int is_write_mode = (isMatchingString("w", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("s", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("write", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("set", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("post", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("put", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("place", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("rpleac", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("replace", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("save", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("upload", mode_arg, MBUFFER_SIZE) ||
                       isMatchingString("overwrite", mode_arg, MBUFFER_SIZE));
  int is_file_mode = (isMatchingString("f", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("l", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("from", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("file", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("load", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("readfile", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("inputfile", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("inputfrom", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("source", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("sourcefile", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("sourcefrom", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("writefromfile", mode_arg, MBUFFER_SIZE));
  int is_read_mode = (isMatchingString("r", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("g", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("read", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("get", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("text", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("download", mode_arg, MBUFFER_SIZE) ||
                      isMatchingString("retrieve", mode_arg, MBUFFER_SIZE));

  if (is_file_mode)
  {
    if (verbose)
      printf("[Verbose] File mode (%s)\n", mode_arg);
    if (argn < 3 || argv[2] == NULL || (argn >= 3 && argv[2][0] == '\0'))
    {
      printf("Error: Missing source file argument to update clipboard from file contents\n");
      return 5;
    }
    if (argn > 3)
    {
      printf("Error: Received too many arguments to update clipboard from file contents\n");
      return 6;
    }
    snprintf(mode_arg, MBUFFER_SIZE, "%s", argv[2]);
    return executeWriteFromFileMode(mode_arg);
  }

  if (is_write_mode)
  {
    if (verbose)
      printf("[Verbose] Write mode (%s)\n", mode_arg);
    if (argn < 2 || argv[2] == NULL)
    {
      printf("Error: Missing target text argument to set clipboard data\n");
      return 4;
    }
    return executeWriteFromArgsMode(argn, argv);
  }

  if (is_read_mode)
  {
    if (verbose)
      printf("[Verbose] Read mode (%s)\n", mode_arg);
    if (argn == 3)
    {
      printf("Error: Received an unexpected argument to reading clipboard content\n");
      return 7;
    }
    if (argn >= 3)
    {
      printf("Error: Received too many arguments to reading clipboard contents\n");
      return 8;
    }
    return executeReadMode();
  }

  printf("Error: Unhandled first mode argument\n");
  return 20;
}
