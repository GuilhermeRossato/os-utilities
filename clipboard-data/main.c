#include "windows.h"
#include "stdio.h"

#define verbose 0

#define BUFFER_SIZE 4096
char buffer[BUFFER_SIZE];

int printHelp(int r)
{
  size_t size = snprintf(
      buffer,
      BUFFER_SIZE - 1,
      "clipboard-data - Utility to manipulate clipboard data\n"
      "\n"
      "Usage:\n"
      "\tclipboard-data --get <format>         Get the clipboard data of the specified format.\n"
      "\tclipboard-data --set <format> <text>  Set the specified format of clipboard data to the string from the program argument.\n"
      "\tclipboard-data --list                 List the formats that contain clipboard data.\n"
      "\tclipboard-data --help                    Display help.\n");
  fwrite(buffer, size, 1, stdout);
  return r;
}

const char *getFormatName(UINT f);
long rawReadClipboardFormatData(UINT format, char *dst_buffer, size_t dst_limit);
long rawSetClipboardDataFormat(UINT format, char *src_buffer, size_t src_size);

// List mode
int iterateAndDisplayClipboardFormatList()
{
  if (0 == OpenClipboard(0))
  {
    return 299; // No open
  }

  UINT f = EnumClipboardFormats(0);

  int clipboard_count = 0;
  int buffer_index = 0;

  buffer[buffer_index] = '[';
  buffer_index++;

  while (f != 0)
  {
    buffer_index += snprintf(
        &buffer[buffer_index],
        BUFFER_SIZE - buffer_index - 1,
        "{\"format\": %d",
        f);
    buffer_index += snprintf(
        &buffer[buffer_index],
        BUFFER_SIZE - buffer_index - 1,
        ", \"name\": \"");
    buffer_index += snprintf(&buffer[buffer_index], BUFFER_SIZE - buffer_index - 1, "%s", getFormatName(f));
    f = f > 0 ? EnumClipboardFormats(f) : 0;
    buffer_index += snprintf(&buffer[buffer_index], BUFFER_SIZE - buffer_index - 1, "%s", f == 0 ? "\"}" : "\"}, ");
    clipboard_count++;
  }
  buffer_index += snprintf(&buffer[buffer_index], BUFFER_SIZE - buffer_index - 1, "]");
  buffer[buffer_index] = '\0';
  buffer_index++;
  fwrite(buffer, buffer_index - 1, 1, stdout);
  CloseClipboard();
  return clipboard_count != 0 ? 0 : 331;
}
// Get mode
int displayClipboardFormatData(UINT format)
{
  long size = rawReadClipboardFormatData(format, buffer, BUFFER_SIZE - 1);
  if (size <= 0)
  {
    printf("Error: Failed with code %d while reading clipboard data of code %d\n", size, format);
    return 1;
  }
  fwrite(buffer, size - 1, 1, stdout);
  return 0;
}
// Set mode
int updateClipboardFormatData(UINT format, char *src_buffer, size_t src_size)
{
  long size = rawSetClipboardDataFormat(format, src_buffer, src_size >= BUFFER_SIZE ? BUFFER_SIZE - 1 : src_size);
  if (size <= 0)
  {
    printf("Error: Failed with code %d while writing clipboard data of code %d\n", size, format);
    return 1;
  }
  fwrite(buffer, size - 1, 1, stdout);
  return 0;
}

int isSeparator(char c)
{
  return (c == '-' || c == '\\' || c == '/' || c == '*' || c == '+');
}

int main(int argn, const char **argv)
{
  if (argn < 2 || argv[0] == NULL || argv[1] == NULL || argv[1][0] == '\0' || argv[1][0] == 'h' || argv[1][0] == 'v')
  {
    return printHelp(1);
  }
  const char *mode = argv[1];
  int start = isSeparator(argv[1][0]) && isSeparator(argv[1][1]) ? 2 : (isSeparator(argv[1][0]) ? 1 : 0);

  int isHelp = mode[start] == 'h' || mode[start] == 'v';
  if (isHelp)
  {
    return printHelp(0);
  }
  int isList = mode[start] == 'l' || mode[start] == 'i';
  if (isList)
  {
    if (argn != 2)
    {
      printf("clipboard-data: Error: Received too many arguments\n");
      return 1;
    }
    return iterateAndDisplayClipboardFormatList();
  }
  int isGet = mode[start] == 'g' || mode[start] == 'r';
  int isSet = mode[start] == 's' || mode[start] == 'w';
  if (!isGet && !isSet)
  {
    printf("clipboard-data: Error: Unhandled or unknown mode argument (expected \"--get\", \"--set\", or \"--list\")\n");
    return 1;
  }
  if (argn < 3)
  {
    printf("clipboard-data: Error: Clipboard data format code not specified\n");
    return 1;
  }

  // Copy argument to mutable buffer to operate on it safely
  char *valueStr = calloc(64, sizeof(char));
  if (valueStr == NULL)
  {
    printf("Error: Could not allocate input buffer\n");
    return 1;
  }
  valueStr[0] = '\0';
  valueStr[1] = '\0';
  const char *sourceArg = argv[2];
  int j;
  for (j = 0; j < 64 - 1; j++)
  {
    valueStr[j] = sourceArg[j];
    if (valueStr[j] == '\n' || valueStr[j] == '\r' || valueStr[j] == ' ' || valueStr[j] == '\t' || valueStr[j] == '\0')
    {
      valueStr[j] = '\0';
      valueStr[j + 1] = '\0';
      break;
    }
  }
  if (verbose)
  {
    printf("[Verbose] The target format argument has %d bytes\n", j);
  }
  if (verbose)
  {
    printf("[Verbose] The first target argument character is %d\n", (int)(valueStr[0]));
  }
  if (verbose)
  {
    printf("[Verbose] The full argument string is \"%s\"\n", (char *)valueStr);
  }
  // Get specified format from buffer
  long value = 0;
  char *endPtr = valueStr;
  if ((valueStr[0] == '1' || valueStr[0] == '2' || valueStr[0] == '7') && valueStr[1] == '\0')
  {
    if (verbose)
    {
      printf("[Verbose] Setting argument in special case to %d\n", valueStr[0] - 48);
    }
    value = valueStr[0] == '1' ? 1 : (valueStr[0] == '2' ? 2 : (valueStr[0] - '0'));
  }
  else if (valueStr[0] == '0' && valueStr[1] != '\0' && (valueStr[1] == 'x' || valueStr[1] == 'X'))
  {
    if (verbose)
    {
      printf("[Verbose] Argument has a \"0x\" prefix and must specify a hexadecimal code\n");
    }
    if (valueStr[1] < '0' || (valueStr[1] > 'F' && valueStr[1] <= 'Z') || (valueStr[1] > 'f' && valueStr[1] <= 'z'))
    {
      if (verbose)
      {
        printf("[Verbose] Third argument character is invalid for a hexadecimal code: %d\n", (int)valueStr[1]);
      }
      printf("Error: Invalid clipboard format argument: Expected hexadecimal number but got \"%s\"\n", valueStr);
      return 1;
    }
    value = strtol(valueStr, &endPtr, 16);
    if (verbose)
    {
      printf("[Verbose] Got numeric formatCode from hexadecimal argument: %lu\n", value);
    }
  }
  else
  {
    if (verbose)
    {
      printf("[Verbose] Argument is a simple decimal number\n", value);
    }
    value = strtol(valueStr, &endPtr, 10);
  }
  if (verbose)
  {
    printf("[Verbose] Clipboard format number selected from argument: %lu\n", value);
  }
  if (value <= 0)
  {
    printf("Error: Invalid clipboard format argument: Expected valid number but got \"%s\"\n", valueStr);
    return 1;
  }
  if (verbose)
  {
    printf("[Verbose] Offset from initial pointer after parsing argument is %lu\n", (long)(endPtr - valueStr));
  }
  UINT uFormat = (UINT)((((long)value) <= ((long)0)) ? 0 : value);

  if (isGet)
  {
    if (argn > 3)
    {
      printf("clipboard-data: Error: Received too many arguments to get data\n");
      return 1;
    }
    if (verbose)
    {
      printf("[Verbose] Returning displayClipboardFormatData(%d)\n", uFormat);
    }
    return displayClipboardFormatData(uFormat);
  }

  if (argn > 4)
  {
    printf("clipboard-data: Error: Received too many arguments to set data\n");
    return 1;
  }
  if (argn < 4)
  {
    printf("clipboard-data: Error: Data to be write to clipboard not specified\n");
    return 1;
  }

  size_t i;
  for (i = 0; i < BUFFER_SIZE; i++)
  {
    buffer[i] = argv[3][i];
    if (buffer[i] == '\0')
    {
      break;
    }
  }

  if (verbose)
  {
    printf("[Verbose] Returning setClipboardFormatData(%d, buffer, %lld)\n", uFormat, i);
  }
  return updateClipboardFormatData(uFormat, buffer, i + 1);
}

const char *getFormatName(UINT f)
{
  switch (f)
  {
  case 0:
    return "(EMPTY)";
  case CF_TEXT:
    return "CF_TEXT";
  case CF_BITMAP:
    return "CF_BITMAP";
  case CF_METAFILEPICT:
    return "CF_METAFILEPICT";
  case CF_SYLK:
    return "CF_SYLK";
  case CF_DIF:
    return "CF_DIF";
  case CF_TIFF:
    return "CF_TIFF";
  case CF_OEMTEXT:
    return "CF_OEMTEXT";
  case CF_DIB:
    return "CF_DIB";
  case CF_PALETTE:
    return "CF_PALETTE";
  case CF_PENDATA:
    return "CF_PENDATA";
  case CF_RIFF:
    return "CF_RIFF";
  case CF_WAVE:
    return "CF_WAVE";
  case CF_UNICODETEXT:
    return "CF_UNICODETEXT";
  case CF_ENHMETAFILE:
    return "CF_ENHMETAFILE";
  case CF_HDROP:
    return "CF_HDROP";
  case CF_LOCALE:
    return "CF_LOCALE";
  case CF_DIBV5:
    return "CF_DIBV5";
  case CF_MAX:
    return "CF_MAX";
  case CF_OWNERDISPLAY:
    return "CF_OWNERDISPLAY";
  case CF_DSPTEXT:
    return "CF_DSPTEXT";
  case CF_DSPBITMAP:
    return "CF_DSPBITMAP";
  case CF_DSPMETAFILEPICT:
    return "CF_DSPMETAFILEPICT";
  case CF_DSPENHMETAFILE:
    return "CF_DSPENHMETAFILE";
  case CF_PRIVATEFIRST:
    return "CF_PRIVATEFIRST";
  case CF_PRIVATELAST:
    return "CF_PRIVATELAST";
  case CF_GDIOBJFIRST:
    return "CF_GDIOBJFIRST";
  case CF_GDIOBJLAST:
    return "CF_GDIOBJLAST";
  default:
    return "CUSTOM";
  }
}

long rawReadClipboardFormatData(UINT format, char *dst_buffer, size_t dst_limit)
{
  if (format == 0 || dst_buffer == NULL || dst_limit <= 3)
  {
    printf("Error: Invalid format or insufficient arguments\n");
    return -332;
  }

  dst_buffer[0] = '\0';
  dst_buffer[1] = '\0';

  if (0 == OpenClipboard(0))
  {
    printf("Error: OpenClipboard failed\n");
    return -341;
  }

  if (verbose)
    printf("[Verbose] OpenClipboard was successful\n");

  HANDLE handle = GetClipboardData(format);

  if (handle == NULL)
  {
    if (verbose)
      printf("[Verbose] GetClipboardData returned NULL and CloseClipboard returned %d\n", CloseClipboard());
    else
      CloseClipboard();
    printf("Error: GetClipboardData failed to retrieve handle for clipboard format of code %d\n", format);
    return -356;
  }

  if (verbose)
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
    return -376;
  }

  if (verbose)
    printf("[Verbose] GlobalLock was successful\n");

  size_t copy_size = src_size < dst_limit ? src_size : dst_limit;

  if (verbose)
    printf("[Verbose] Copying %llu bytes to target buffer\n", copy_size);

  dst_buffer[0] = '\0';
  dst_buffer[1] = '\0';
  dst_buffer[copy_size - 2] = '\0';
  dst_buffer[copy_size - 1] = '\0';
  memcpy(dst_buffer, src_buffer, copy_size);
  if (dst_limit < copy_size)
  {
    dst_buffer[copy_size] = '\0';
  }

  if (0 == GlobalUnlock(handle))
  {
    printf("Error: GlobalUnlock failed\n");
    return -396;
  }

  if (verbose)
    printf("[Verbose] GlobalUnlock was successful and CloseClipboard returned %d\n", CloseClipboard());
  else
    CloseClipboard();

  return (long)copy_size;
}

long rawSetClipboardDataFormat(UINT format, char *src_buffer, size_t src_size)
{
  if (format == 0 || src_buffer == NULL || src_size <= 0)
  {
    printf("Error: Invalid or insufficient arguments to set clipboard data\n");
    return -418;
  }

  if (verbose)
    printf("[Verbose] Allocating global memory of size %llu\n", src_size);

  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, src_size);

  if (handle == NULL)
  {
    if (verbose)
      printf("[Verbose] GlobalAlloc returned NULL handle using GMEM_MOVABLE flag\n");

    printf("Error: GlobalAlloc failed to allocate %llu bytes\n", src_size);
    return -432;
  }

  char *dst_buffer = GlobalLock(handle);
  if (dst_buffer == NULL)
  {
    if (verbose)
      printf("[Verbose] GlobalLock returned NULL pointer\n");

    printf("Error: GlobalLock failed to return clipboard buffer\n");
    return -442;
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
    return -452;
  }

  if (verbose)
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
