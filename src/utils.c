#include <stdlib.h>

#include "utils.h"

char* get_buffer_line(char* buffer, char** last_char) {
  char* line, * ptr = buffer;
  int line_size = 0;

  // find size line length ( check for \r\n string [CRLF])
  while (*ptr != '\r' || *(ptr + 1) != '\n') {
    ptr++;
    line_size++;
  }

  if (!line_size) return NULL;

  line = malloc(line_size);
  ptr = buffer;

  for (int i = 0; i < line_size; i++) {
    line[i] = *ptr;
    ptr++;
  }
  line[line_size] = '\0';

  *last_char = ptr + 2;

  return line;
}