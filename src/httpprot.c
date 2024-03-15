#include "httpprot.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SMALL_BUFLEN 64

// defines size of static elements in http response
#define _RESP_FIXED_SIZE 6

#define CRLF "\r\n"
// convert ASCII character to integer
#define char_to_int(c) (c) - 48

void http_header_free(http_header* header) {
  free(header->value);
  header->value = NULL;
}

void http_response_free(char* response) {
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

    strncpy_s(temp, SMALL_BUFLEN, pstart, len);

    // compare
    if (strcmp(temp, value) == 0) {
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
      // so the header isnt found
      break;
    }

    // header name length
    len = ptr - line;

    if (len >= SMALL_BUFLEN) {
      break;
    }

    strncpy_s(temp, SMALL_BUFLEN, line, len);

    if (strcmp(temp, header_name) == 0) {
      if (header_value) {
        // increase pointer so it points to the header value
        do {
          ptr++;
        } while (*ptr == ' ');

        char* end_of_value = str_find_char(ptr, '\0');

        len = end_of_value - ptr;

        header_value = malloc(len + 1);

        // set header value
        strncpy_s(header_value, len + 1, ptr, len);
        header_value[len] = '\0';
      }
      return 1;
    }
  }

  return 0;
}

int _total_headers_size(const http_header* const headers, int h_count) {
  int tsize = 0;

  for (int i = 0; i < h_count; i++) {
    tsize += strlen(headers[i].name) + strlen(headers[i].value) + 4 /* '\r\n' and ': ' */;
  }

  return tsize;
}

char* build_http_response
(
  const char* const http_variant,
  http_version version,
  const char* const status,
  const http_header* const headers,
  int h_count,
  char* const body,
  int* const response_size
) {
  int resp_size = _RESP_FIXED_SIZE +
    strlen(http_variant) + strlen(status) +
    _total_headers_size(headers, h_count) + strlen(body);

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
  strcat_s(response, resp_size, http_variant);

  // construct version string
  char temp[8];
  if (version.minor == 0) {
    sprintf_s(temp, sizeof(temp), "/%d ", version.major);
  }
  else {
    // <major>.<minor>
    sprintf_s(temp, sizeof(temp), "/%d.%d ", version.major, version.minor);
  }

  // concat version string to response
  strcat_s(response, resp_size, temp);

  // concat status string to respone
  strcat_s(response, resp_size, status);
  // end line
  strcat_s(response, resp_size, CRLF);

  // concat all headers to response
  for (int i = 0; i < h_count; i++) {
    // add header content
    strcat_s(response, resp_size, headers[i].name);
    strcat_s(response, resp_size, ": ");
    strcat_s(response, resp_size, headers[i].value);
    // end line
    strcat_s(response, resp_size, CRLF);
  }

  // add empty line to indicate start of message body
  strcat_s(response, resp_size, CRLF);

  // add msg body
  strcat_s(response, resp_size, body);

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

  // copy string to next space to the buffer
  strncpy_s(temp, SMALL_BUFLEN, buffer, len);

  // set request method
  if (strcmp(temp, "GET") == 0) {
    result->method = HTTP_GET;
  }
  else if (strcmp(temp, "POST") == 0) {
    result->method = HTTP_POST;
  }
  else if (strcmp(temp, "PUT") == 0) {
    result->method = HTTP_PUT;
  }
  else if (strcmp(temp, "HEAD") == 0) {
    result->method = HTTP_HEAD;
  }
  else if (strcmp(temp, "CONNECT") == 0) {
    result->method = HTTP_CONNECT;
  }
  else if (strcmp(temp, "DELETE") == 0) {
    result->method = HTTP_DELETE;
  }
  else if (strcmp(temp, "OPTIONS") == 0) {
    result->method = HTTP_OPTIONS;
  }
  else if (strcmp(temp, "PATCH") == 0) {
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

  // copy string to next space to the resource buffer
  strncpy_s(result->url_path, _PATHSIZE_, ptr, len);

  // find next slash
  if (!(ptr = strchr(++ptr2, '/'))) {
    return -1;
  }

  char version_buffer[SMALL_BUFLEN];

  // copy version string
  ptr++;
  int c = 0;

  while (*ptr != ' ' && (*ptr != '\r' || *(ptr + 1) != '\n') && *ptr != '\0') {
    version_buffer[c++] = *ptr++;
  }

  if (*ptr == '\0')
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
  if (!(result->content = get_buffer_line(ptr, NULL, 0))) {
    return -5;
  }

  return 0;
}