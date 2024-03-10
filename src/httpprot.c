#include "httpprot.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SMALL_BUFLEN 64
#define LINE_LEN 256

// defines size of static elements in http response, e.g. HTTP/1.1 is size of 8
#define _RESP_FIXED_SIZE 14

#define CRLF "\r\n"
// convert ASCII character to integer
#define char_to_int(c) (c) - 48


void http_header_free(http_header* header) {
  free(header->value);
  header->value = NULL;
}

int is_in_header(const char* const header_value, char delim, char* str) {
  char temp[SMALL_BUFLEN];
  int len = 0, quit = 0;
  const char* pstart = header_value, * pend;

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

int find_http_header(char* const buffer, http_header* const header) {
  char line[LINE_LEN];
  char temp[SMALL_BUFLEN];
  char* lptr = buffer;
  char* ptr;
  int len = 0;

  while (lptr = get_buffer_line(lptr, line, LINE_LEN)) {
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

int _total_headers_size(http_header* headers, int h_count) {
  int tsize = 0;

  for (int i = 0; i < h_count; i++) {
    tsize += strlen(headers[i].name) + strlen(headers[i].value) + 4 /* '\r\n' and ': ' */;
  }

  return tsize;
}

char* build_http_response
(http_version version, char* status, http_header* headers, int h_count, char* body) {
  int response_size =
    _RESP_FIXED_SIZE + strlen(status) + _total_headers_size(headers, h_count) + strlen(body);

  char* response = malloc(response_size);
  // mark as empty
  *response = '\0';

  // construct version string
  char temp[10];
  sprintf_s(temp, sizeof(temp), "HTTP/%d.%d ", version.major, version.minor);

  // concat version string to response
  strcat_s(response, response_size, temp);
  // concat status string to respone
  strcat_s(response, response_size, status);
  // end line
  strcat_s(response, response_size, CRLF);

  // concat all headers to response
  for (int i = 0; i < h_count; i++) {
    // add header content
    strcat_s(response, response_size, headers[i].name);
    strcat_s(response, response_size, ": ");
    strcat_s(response, response_size, headers[i].value);
    // end line
    strcat_s(response, response_size, CRLF);
  }

  // add empty line to indicate start of message body
  strcat_s(response, response_size, CRLF);

  // add msg body
  strcat_s(response, response_size, body);

  return response;
}

int resolve_http_request_line(char* const line_buffer, http_request_line* result) {
  char* ptr, * ptr2;
  int len = 0;
  char temp[SMALL_BUFLEN];

  // find next space
  ptr = strchr(line_buffer, ' ');
  len = ptr - line_buffer;
  if (len + 1 > SMALL_BUFLEN)
    return -1;

  // copy string to next space to the buffer
  strncpy_s(temp, SMALL_BUFLEN, line_buffer, len);

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
  if (len + 1 > _PATHSIZE)
    return -3;

  // copy string to next space to the resource buffer
  strncpy_s(result->path, SMALL_BUFLEN, ptr, len);

  // find dot
  ptr = strchr(++ptr2, '.');

  // set version
  result->version.major = char_to_int(*(ptr - 1));
  result->version.minor = char_to_int(*(ptr + 1));
  if (result->version.major < 0 || result->version.minor < 0)
    return -4;

  return 0;
}

void extract_file_extension(const char* const filepath, char* const extension) {
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

int get_resource(char* path, int max_path_size, http_version version, char* req_headers_buffer, char* response) {
  http_header h_accept;
  h_accept.name = "Accept";

  if (!find_http_header(req_headers_buffer, &h_accept)) {
    printf("http parsing error\n");
    return -1;
  }

  // if root is requested then get index.html
  if (path[strlen(path) - 1] == '/') {
    strcat_s(path, max_path_size, "index.html");
  }

  char extension[8];
  extract_file_extension(path, extension);

  if (strcmp(extension, "html") == 0) {
    if (!is_in_header(h_accept.value, ',', "text/html")) {
      printf("http parsing error\n");
      return -2;
    }
  }
  else if (strcmp(extension, "json") == 0) {

  }

  http_header_free(&h_accept);

  // open file
  FILE* fp;
  fopen_s(&fp, path, "rb");

  if (!fp) {
    printf("failed to open resource file: %s\n", path);
    return -3;
  }

  fseek(fp, 0, SEEK_END);
  int file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* file_buf = malloc(file_size + 1);

  if (fread(file_buf, sizeof(char), file_size, fp) != file_size) {
    printf("Error while reading resource file: %s\n", path);
    return -3;
  }
  file_buf[file_size] = '\0';

  char buff[16];

  _itoa_s(file_size, buff, 16, 10);

  http_header resp_headers[] = {
    { "Content-Length", buff },
    { "Content-Type", "text/html; charset=utf-8" },
  };
  int response_headers_count = sizeof(resp_headers) / sizeof(http_header);

  response = build_http_response(version, "200 OK",
    resp_headers, response_headers_count, file_buf);

  free(file_buf);
  fclose(fp);

  return response_headers_count;
}