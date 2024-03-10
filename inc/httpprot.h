#pragma once

#define HTTP_GET 1
#define HTTP_POST 2
#define HTTP_PUT 3
#define HTTP_HEAD 4
#define HTTP_DELETE 5
#define HTTP_CONNECT 6
#define HTTP_OPTIONS 7
#define HTTP_PATCH 8

#define _PATHSIZE 64

typedef int req_method;

typedef struct http_version {
  short major;
  short minor;
} http_version;

typedef struct http_request {
  req_method method;
  http_version version;
  char path[_PATHSIZE];
} http_request;

// must be freed with http_header_free()
typedef struct http_header {
  char* name;
  char* value;
} http_header;


void http_header_free(http_header* header);

// header 'name' field must be filled before
// header 'value' field is filled by the function
// returns boolean indicating whether the header is present
int find_http_header_value(char* const buffer, http_header* const header);

// returns boolean indicating whether the string is present
int is_in_header_value(const char* const header_value, char delim, char* str);

int _total_headers_size(http_header* headers, int h_count);

/// @brief builds http request from specified parameters
/// @param version http version
/// @param status example of status: "200 OK"
/// @param headers pointer to http_header linked list
/// @param body pointer to message body
/// @returns pointer to
char* build_http_response
(http_version version, char* status, http_header* headers, int h_count, char* body);

/// @brief resolves http request in specified buffer
/// @param line_buffer buffer containing http request to be resolved
/// @param result resolved http request
/// @return error code
int resolve_http_request(char* const line_buffer, http_request* result);