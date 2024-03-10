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

typedef struct http_request_line {
  req_method method;
  http_version version;
  char path[_PATHSIZE];
} http_request_line;

// must be freed with http_header_free()
typedef struct http_header {
  char* name;
  char* value;
} http_header;

/// @brief frees the header memory
/// @param header pointer to the header
void http_header_free(http_header* header);

/// @brief looks for value of given http header name and sets it in header->value
/// @param buffer buffer containg http headers
/// @param header pointer to http_header struct;
///        header->name defines name of the header to search for;
///        header->value contains found value (if found)
/// @return 1 if found, 0 if not
int find_http_header(char* const buffer, http_header* const header);

/// @brief checks if given string is a value of given http header content
/// @param header_value header content (value)
/// @param delim sign which separates values in header content
/// @param str value to search for
/// @return 1 if found, 0 if not
int is_in_header(const char* const header_value, char delim, char* str);


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
int resolve_http_request_line(char* const line_buffer, http_request_line* result);

/// @brief extracts file extension from given file path
/// @param filepath path of the file
/// @param extension file extension of filepath in string format
void extract_file_extension(const char* const filepath, char* const extension);

/// @brief prepares http response for GET request
/// @param path path to the requested resource
/// @param max_path_size maximum number of characters 'path' can contain
/// @param version http version
/// @param req_headers pointer to string containing http request headers
/// @param response resulting http response
/// @return error code if less than 0, otherwise response length
int get_resource(char* path, int max_path_size, http_version version, char* req_headers_buffer, char* const response);