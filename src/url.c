#include "misc/url.h"

#include <stdlib.h>
#include <string.h>

void free_url_parts(url_parts* url) {
  free(url->path);
  free(url->query_string);
}

void url_append(char* const url, int max_url_size, const char* const string_to_append) {
  // check if there is a query string
  char* query_sign = strchr(url, '?');
  if (query_sign) {
    // replace query sign with \0 (basically 'remove' query string)
    *query_sign = '\0';
  }

  // don't double slashes
  if (url[strlen(url) - 1] == '/' && string_to_append[0] == '/') {
    strcat_s(url, max_url_size, string_to_append + 1);
  }
  else {
    strcat_s(url, max_url_size, string_to_append);
  }
}

// ............

void url_parse(char* const url, url_parts* const parsed_url) {
  char* ptr = url, * ptr2 = url;
  int count = 0;

  while (*ptr != ':' || !(*ptr)) {
    count++;
    ptr++;
  }

  if (!(*ptr)) {

  }

  strncpy_s(parsed_url->protocol, _URL_PROT_SIZE, url, count);

  // ...
}
