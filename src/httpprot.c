#include "server/httpprot.h"

#include "misc/utils.h"
#include "cp_defs/str.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SMALL_BUFLEN 64

// defines size of static elements in http response
#define _RESP_FIXED_SIZE 7

#define CRLF "\r\n"
// convert ASCII character to integer
#define char_to_int(c) (c) - 48

void http_response_free(http_response response) {
  free(response);
  response = NULL;
}

int http_is_value_in_header(const http_header* const header, char delim, char* value) {
  char temp[SMALL_BUFLEN];
  int len = 0, quit = 0;
  const char* pstart = header->value, * pend;

  do {
    if (!(pend = str_find_char(pstart, delim))) { // find end of header value item
      pend = str_find_char(pstart, '\0');
      quit = 1;
    }

    len = pend - pstart;

    if (len >= SMALL_BUFLEN) {
      return 0;
    }

    STRNCPY(temp, SMALL_BUFLEN, pstart, len);

    // compare
    if (str_is_equal(temp, value)) {
      return 1;
    }
    else {
      pend++;
      // go to next non-space character
      while (*pend == ' ') {
        pend++;
      }
      pstart = pend;
    }
  } while (!quit);

  return 0;
}

int http_get_header
(char* const buffer, const char* const header_name, char* header_value) {
  if (!buffer || !header_name) return 0;

  char line[_HEADERLINESIZE_];
  char temp[SMALL_BUFLEN];
  char* lptr = buffer, * ptr;
  int len = 0;

  while (lptr = get_buffer_line(lptr, line, _HEADERLINESIZE_)) {
    if (!(ptr = str_find_char(line, ':'))) { // find end of header name
      // if there is no header name in the line, it means that message body is next
      // so the header isn't found
      break;
    }

    // calc header name length
    len = ptr - line;

    if (len >= SMALL_BUFLEN) {
      break;
    }

    STRNCPY(temp, SMALL_BUFLEN, line, len);

    if (str_is_equal(temp, header_name)) {
      if (header_value) {
        // increase pointer so it points to the header value (not spaces)
        do {
          ptr++;
        } while (*ptr == ' ');

        char* end_of_value = str_find_char(ptr, '\0');

        len = end_of_value - ptr;

        header_value = malloc(len + 1);

        // set header value
        STRNCPY(header_value, len + 1, ptr, len);
        header_value[len] = '\0';
      }
      return 1;
    }
  }

  return 0;
}

int _total_headers_size(const http_header* headers, int h_count) {
  int tsize = 0;

  for (int i = 0; i < h_count; i++) {
    tsize += str_length(headers[i].name) + str_length(headers[i].value) + 4 /* '\r\n' and ': ' */;
  }

  return tsize;
}

int http_header_array_resize
(http_header* header_array, int current_size, int new_size) {
  if (!header_array || new_size <= current_size) return 0;

  http_header* arr = malloc(sizeof(http_header) * new_size);

  for (int i = 0; i < current_size; i++) {
    arr[i] = header_array[i];
  }

  http_header_array_destroy(header_array);
  header_array = arr;
  return 1;
}

http_header* http_header_array_create
(int arr_size, int init_list_size, http_header* init_list) {
  if (arr_size < 1 || init_list_size > arr_size) return NULL;

  http_header* result = malloc(sizeof(http_header) * arr_size);

  for (int i = 0; i < init_list_size; i++) {
    result[i] = init_list[i];
  }

  return result;
}

int http_header_array_push(
  int header_array_size, int header_array_length, http_header* header_array,
  int headers_size, http_header* headers
) {
  if (!header_array || !headers || header_array_length > header_array_size || headers_size < 1) return -1;

  int total_new_length = header_array_length + headers_size;

  if (total_new_length > header_array_size) {
    http_header_array_resize(header_array, header_array_size, total_new_length);
  }

  for (int i = header_array_length; i < total_new_length; i++) {
    header_array[i] = headers[i - header_array_length];
  }

  return total_new_length;
}

void http_header_array_destroy(http_header* array) {
  free(array);
  array = NULL;
}

http_response http_build_response(
  const char* const http_variant,
  http_version version,
  const char* const status,
  const http_header* headers,
  int h_count,
  char* const body,
  int* const response_size
) {
  if (!http_variant || !status) return NULL;

  int resp_size = _RESP_FIXED_SIZE +
    str_length(http_variant) + str_length(status) +
    _total_headers_size(headers, h_count) + str_length(body);

  if (version.minor == 0) {
    resp_size += 1; // <major>
  }
  else {
    resp_size += 3; // <major>.<minor>
  }

  char* response = malloc(resp_size);
  // mark as empty
  *response = '\0';

  // concat http variant to response
  STRCAT(response, resp_size, http_variant);

  // construct version string
  char temp[8];
  if (version.minor == 0) {
    SPRINTF(temp, sizeof(temp), "/%d ", version.major);
  }
  else {
    // <major>.<minor>
    SPRINTF(temp, sizeof(temp), "/%d.%d ", version.major, version.minor);
  }

  // concat version string to response
  STRCAT(response, resp_size, temp);

  // concat status string to respone
  STRCAT(response, resp_size, status);
  // end line
  STRCAT(response, resp_size, CRLF);

  for (int i = 0; i < h_count; i++) {
    // add header content
    STRCAT(response, resp_size, headers[i].name);
    STRCAT(response, resp_size, ": ");
    STRCAT(response, resp_size, headers[i].value);
    // end line
    STRCAT(response, resp_size, CRLF);
  }

  // add empty line to indicate start of message body
  STRCAT(response, resp_size, CRLF);

  // add msg body
  if (body)
    STRCAT(response, resp_size, body);

  if (response_size)
    *response_size = resp_size;

  return response;
}

int resolve_http_request_line(char* const buffer, http_request* result) {
  char* ptr, * ptr2;
  int len = 0;
  char temp[SMALL_BUFLEN];
  // find next space
  if (!(ptr = strchr(buffer, ' '))) {
    return -1;
  }

  len = ptr - buffer;

  if (len >= SMALL_BUFLEN)
    return -2;

  // copy req method string to the temp buffer
  STRNCPY(temp, SMALL_BUFLEN, buffer, len);
  printf("%s", temp); // error
  // set request method
  if (str_is_equal(temp, "GET")) {
    result->method = HTTP_GET;
  }
  else if (str_is_equal(temp, "POST")) {
    result->method = HTTP_POST;
  }
  else if (str_is_equal(temp, "PUT")) {
    result->method = HTTP_PUT;
  }
  else if (str_is_equal(temp, "HEAD")) {
    result->method = HTTP_HEAD;
  }
  else if (str_is_equal(temp, "CONNECT")) {
    result->method = HTTP_CONNECT;
  }
  else if (str_is_equal(temp, "DELETE")) {
    result->method = HTTP_DELETE;
  }
  else if (str_is_equal(temp, "OPTIONS")) {
    result->method = HTTP_OPTIONS;
  }
  else if (str_is_equal(temp, "PATCH")) {
    result->method = HTTP_PATCH;
  }
  else {
    return -3;
  }

  // find next space
  if (!(ptr2 = strchr(++ptr, ' '))) {
    return -1;
  }

  len = ptr2 - ptr;

  if (len >= _PATHSIZE_)
    return -3;

  // copy url string to the url_path container
  STRNCPY(result->url_path, _PATHSIZE_, ptr, len);

  // find next slash
  if (!(ptr = strchr(++ptr2, '/'))) {
    return -1;
  }

  char version_buffer[SMALL_BUFLEN];
  // copy version string
  char* vptr = ptr + 1;
  int c = 0;

  while (*vptr != ' ' && (*vptr != '\r' || *(vptr + 1) != '\n') && *vptr != '\0') {
    version_buffer[c++] = *vptr++;
  }
  version_buffer[c] = '\0';

  if (*vptr == '\0')
    return -4;

  // copy variant
  c = 0;
  while (ptr2 != ptr) {
    result->variant[c++] = *ptr2++;
  }
  result->variant[c] = '\0';

  // set version
  result->version.major = char_to_int(version_buffer[0]);
  if (version_buffer[1] == '.') {
    result->version.minor = char_to_int(version_buffer[2]);
  }
  else {
    result->version.minor = 0;
  }

  if (result->version.major < 0 || result->version.minor < 0)
    return -4;

  // set pointer to http request content
  if (!(result->raw_content = get_buffer_line(ptr, NULL, 0))) {
    return -5;
  }

  return 0;
}