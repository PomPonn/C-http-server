#include "utils.h"

#include <string.h>

char* get_buffer_line(char* strbuf, char* linebuf, unsigned int linebuff_size) {
  if (!strbuf || !linebuf || linebuff_size == 0) return NULL;

  char* end = strbuf;
  unsigned int line_size = 0;

  // find line length ( check for \r\n string [CRLF])
  while ((*end != '\r' || *(end + 1) != '\n') && *end != '\0') {
    end++;
    line_size++;
  }

  if (!line_size || line_size >= linebuff_size) return NULL;

  strncpy_s(linebuf, linebuff_size, strbuf, line_size);

  end = end + 2;

  return end;
}