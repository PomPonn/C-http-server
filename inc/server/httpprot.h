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

// user might redefine these macros
#define _PATHSIZE_ 256
#define _HEADERLINESIZE_ 512
#define _VARIANTSIZE_ 64

typedef int req_method;
typedef char* http_response;

typedef struct http_version {
  short major;
  short minor;
} http_version;

typedef struct http_request {
  req_method method;
  char variant[_VARIANTSIZE_];
  http_version version;
  char url_path[_PATHSIZE_];
  char* raw_content;
} http_request;

typedef struct http_header {
  char* name;
  char* value;
} http_header;

/// @brief frees allocated http response
/// @param response pointer to http response
void http_response_free(http_response response);

int http_header_array_resize(http_header* header_array,
  int current_size, int new_size);

// [optional] creates dynamic header array (use when you need variable length header array)
// init_list param must be of array_size
http_header* http_header_array_create
(int array_size, int init_list_size, http_header* init_list);

/// @brief push new item to exisiting header array
/// @param header_array header array to work with
/// @param header_array_size allocated size of header array
/// @param header_array_length true length of header array
/// @param headers_size length of headers to push
/// @param headers new headers to push
/// @return new array length
int http_header_array_push
(
  int header_array_size, int header_array_length, http_header* header_array,
  int headers_size, http_header* headers
);

void http_header_array_destroy(http_header* array);

/// @brief looks for value of given http header name and sets it in header->value
/// @param buffer buffer containg http headers
/// @param header_name defines name of the header to search for
/// @param header_value contains allocated found value (if found, header should be freed with http_header_free)
/// @return 1 if found, 0 if not
int http_get_header
(char* const buffer, const char* const header_name, char* const header_value);

/// @brief checks if given string is a value of given http header content
/// @param header_value header content (value)
/// @param delim sign which separates values in header content
/// @param value value to search for
/// @return 1 if found, 0 if not
int http_is_value_in_header(const http_header* const header, char delim, char* value);

/// @brief calculates total size of headers after converting them to string
int _total_headers_size(const http_header* headers, int h_count);

/// @brief builds http response from specified parameters
/// @param http_variant http or https
/// @param version http version
/// @param status example of status: "200 OK" '<status code> <status info>'
/// @param headers pointer to `http_header` array
/// @param h_count `http_header` array length
/// @param body pointer to message body
/// @param response_size stores size of built response
/// @returns pointer to built http response
char* http_build_response(
  const char* const http_variant,
  http_version version,
  const char* const status,
  const http_header* headers,
  int headers_count,
  char* const body,
  int* const response_size
);

/// @brief resolves http request line in specified buffer
/// @param line_buffer buffer containing request
/// @param result resolved http request line
/// @return error code -> 0 if successful
int resolve_http_request_line(char* const buffer, http_request* result);

/// @brief extracts file extension from given file url_path
/// @param filepath url_path of the file
/// @param extension file extension of filepath in string format
void get_file_extension(const char* const filepath, char* const extension);