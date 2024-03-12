#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* get_buffer_line(char* strbuf, char* linebuf, unsigned int linebuff_size) {
  if (!strbuf) return NULL;

  char* end = strbuf;
  unsigned int line_size = 0;

  // find line length ( check for \r\n string [CRLF])
  while ((*end != '\r' || *(end + 1) != '\n') && *end != '\0') {
    end++;
    line_size++;
  }

  if (!line_size || (line_size >= linebuff_size && linebuf)) return NULL;

  if (linebuf) {
    strncpy_s(linebuf, linebuff_size, strbuf, line_size);
    linebuf[line_size] = '\0';
  }

  end = end + 2;

  return end;
}

int get_resource(const char* const path, char** const content) {
  // open file
  FILE* fp;
  fopen_s(&fp, path, "rb");

  if (!fp) {
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  int file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *content = malloc(file_size);

  if (fread(*content, sizeof(char), file_size, fp) != file_size) {
    return -2;
  }

  fclose(fp);
  return file_size;
}

void free_resource(char* content) {
  free(content);
  content = NULL;
}