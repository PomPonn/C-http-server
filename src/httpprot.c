#include "httpprot.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SMALL_BUFLEN 64

// defines size of static elements in http response, e.g. HTTP/1.1 is size of 8
#define _RESP_FIXED_SIZE 14

#define CRLF "\r\n"
// convert ASCII character to integer
#define char_to_int(c) (c) - 48


void http_header_free(http_header* header) {
  free(header->value);
  header->value = NULL;
}

void free_http_response(char* response) {
  free(response);
  response = NULL;
}

int is_in_http_header(const http_header* const header, char delim, char* str) {
  char temp[SMALL_BUFLEN];
  int len = 0, quit = 0;
  const char* pstart = header->value, * pend;

  do {
    pend = strchr(pstart, delim); // find end of header value item
    // check if strchr succeeded
    if (!pend) {
      pend = strchr(pstart, '\0');
      quit = 1;
    }
    len = pend - pstart;

    if (len + 1 > SMALL_BUFLEN) {
      return 0;
    }

    strncpy_s(temp, SMALL_BUFLEN, pstart, len);

    // compare
    if (strcmp(temp, str) == 0) {
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

int get_http_header_value(char* const buffer, http_header* const header) {
  char line[_HEADERLINESIZE_];
  char temp[SMALL_BUFLEN];
  char* lptr = buffer;
  char* ptr;
  int len = 0;

  while (lptr = get_buffer_line(lptr, line, _HEADERLINESIZE_)) {
    ptr = strchr(line, ':'); // find end of header name
    len = ptr - line;
    if (len + 1 > SMALL_BUFLEN) {
      return 0;
    }

    strncpy_s(temp, SMALL_BUFLEN, line, len);
    if (strcmp(temp, header->name) == 0) {
      ptr += 2; // increase pointer so it points to the header value

      char* end_of_value = strchr(ptr, '\0');
      len = end_of_value - ptr;

      header->value = malloc(len + 1);

      // set header value
      strncpy_s(header->value, len + 1, ptr, len);
      header->value[len] = '\0';

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
  http_version version,
  char* status,
  const http_header* const headers,
  int h_count,
  char* body,
  int* response_size
) {
  int resp_size =
    _RESP_FIXED_SIZE + strlen(status) + _total_headers_size(headers, h_count) + strlen(body);

  char* response = malloc(resp_size);
  // mark as empty
  *response = '\0';

  // construct version string
  char temp[10];
  sprintf_s(temp, sizeof(temp), "HTTP/%d.%d ", version.major, version.minor);

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
  ptr = strchr(buffer, ' ');
  len = ptr - buffer;
  if (len + 1 > SMALL_BUFLEN)
    return -1;

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
    return -2;
  }

  // find next space
  ptr2 = strchr(++ptr, ' ');
  len = ptr2 - ptr;
  if (len + 1 > _HEADERLINESIZE_)
    return -3;

  // copy string to next space to the resource buffer
  strncpy_s(result->url_path, SMALL_BUFLEN, ptr, len);

  // find dot
  ptr = strchr(++ptr2, '.');

  // set version
  result->version.major = char_to_int(*(ptr - 1));
  result->version.minor = char_to_int(*(ptr + 1));
  if (result->version.major < 0 || result->version.minor < 0)
    return -4;

  // set pointer to http request content
  if (!(result->content = get_buffer_line(ptr, NULL, 0))) {
    return -5;
  }

  return 0;
}

void get_file_extension(const char* const filepath, char* const extension) {
  // find last dot
  char* pptr = strrchr(filepath, '.'), * eptr = extension;
  pptr++;

  while (*pptr)
  {
    *eptr = *pptr;
    eptr++;
    pptr++;
  }
  *eptr = '\0';
}