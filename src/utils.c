#include "misc/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32

#include <Windows.h>

#elif __linux__

#include <unistd.h>

#endif

int get_working_directory(char* buffer, int buffer_size);

char* str_find_char(const char* str, const char c) {
  return strchr(str, c);
}

char* str_find_char_reversed(const char* str, const char c) {
  return strrchr(str, c);
}

void int_to_string(int value, char* const buffer, int buffer_size) {
  _itoa_s(value, buffer, buffer_size, 10);
}

int str_length(const char* str) {
  return str ? strlen(str) : 0;
}

int str_compare(const char* str1, const char* str2) {
  return strcmp(str1, str2);
}

void str_concat(char* str1, unsigned int byte_size, const char* str2) {
  strcat_s(str1, byte_size, str2);
}
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
    // copy line
    strncpy_s(linebuf, linebuff_size, strbuf, line_size);
    linebuf[line_size] = '\0';
  }

  end = end + 2;

  return end;
}

int str_is_equal(const char* str1, const char* str2) {
  return strcmp(str1, str2) == 0 ? 1 : 0;
}

int read_file(const char* const path, char** const content) {
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
    free(*content);
    *content = NULL;
    return -2;
  }

  fclose(fp);
  return file_size;
}

void free_content(char* content) {
  free(content);
  content = NULL;
}

void get_file_extension(const char* const filepath, char* const extension) {
  char* eptr = extension;
  char* pptr;

  // find last dot
  if (!(pptr = strrchr(filepath, '.'))) {
    *extension = '\0';
    return;
  }

  pptr++;

  while (*pptr)
  {
    *eptr = *pptr;
    eptr++;
    pptr++;
  }
  *eptr = '\0';
}

#ifdef _WIN32

int get_working_directory(char* buffer, int buffer_size) {
  if (!GetCurrentDirectory(buffer_size, buffer)) {
    return 0;
  }
  return 1;
}

#elif __linux__

int get_working_directory(char* buffer, int buffer_size) {
  if (!getcwd(buffer_size, buffer)) {
    return 0;
  }
  return 1;
}

#endif