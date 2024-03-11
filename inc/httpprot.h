#pragma once

// HTTP REQUEST METHODS
#define HTTP_GET 1
#define HTTP_POST 2
#define HTTP_PUT 3
#define HTTP_HEAD 4
#define HTTP_DELETE 5
#define HTTP_CONNECT 6
#define HTTP_OPTIONS 7
#define HTTP_PATCH 8

#define _PATHSIZE 96

typedef int req_method;

typedef struct http_version {
  short major;
  short minor;
} http_version;

typedef struct http_request_line {
  req_method method;
  http_version version;
  char path[_PATHSIZE];
} http_request_line;

typedef struct http_header {
  char* name;
  char* value;
} http_header;

/// @brief frees the header memory
/// @param header pointer to the header
void http_header_free(http_header* header);

/// @brief looks for value of given http header name and sets it in header->value
/// @param buffer buffer containg http headers
/// @param header pointer to `http_header` struct;
///        header->name defines name of the header to search for;
///        header->value contains found value (if found, header should be freed with http_header_free)
/// @return 1 if found, 0 if not
int get_http_header_value(char* const buffer, http_header* const header);

/// @brief checks if given string is a value of given http header content
/// @param header_value header content (value)
/// @param delim sign which separates values in header content
/// @param str value to search for
/// @return 1 if found, 0 if not
int is_in_http_header(const http_header* const header, char delim, char* str);

/// @brief calculates total size of headers after converting to string
int _total_headers_size(const http_header* const headers, int h_count);

/// @brief builds http request from specified parameters
/// @param version http version
/// @param status example of status: "200 OK" '<status code> <status info>'
/// @param headers pointer to `http_header` array
/// @param h_count `http_header` array length
/// @param body pointer to message body
/// @param response_size stores size of built response
/// @returns pointer to built http response
char* build_http_response(
  http_version version,
  char* const status,
  const http_header* const headers,
  int h_count,
  char* const body,
  int* const response_size
);

/// @brief frees allocated http response
/// @param response pointer to http response
void free_http_response(char* response);

/// @brief resolves http request line in specified buffer
/// @param line_buffer buffer containing req line
/// @param result resolved http request line
/// @return error code -> 0 if successful
int resolve_http_request_line(char* const line_buffer, http_request_line* result);

/// @brief extracts file extension from given file path
/// @param filepath path of the file
/// @param extension file extension of filepath in string format
void get_file_extension(const char* const filepath, char* const extension);

/// @brief reads resource content
/// @param path path to the resource
/// @param content resulting resource content
/// @return error code if less than 0, otherwise content length
int get_resource(const char* const path, char** const content);

/// @brief frees allocated resource
/// @param content resource content to free
void free_resource(char* content);