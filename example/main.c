#include "http_server.h"

#include "url.h"
#include "utils.h"
#include "error.h"
#include "structs/record.h"


#define MAX_PATH_SIZE 256
#define MAX_CONTENT_SIZE_STRLEN 64

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT "3021"

#define ROOT_PATH "C:/Users/gangs/OneDrive/Dokumenty/MyStuff/Projects/C_HTTP_server/"

record_str_t mime_types[] = {
  { "html", "text/html; charset=utf-8" },
  { "htm", "text/html" },
  { "css", "text/css" },
  { "json", "text/json" },
  { "js", "text/javascript" },
  { "ico", "image/vnd.microsoft.icon" },
  { "png", "image/png" },
  { "jpg", "image/jpeg" },
  { "jpeg", "image/jpeg" },
};

void on_request(http_request* req, http_response* res) {
  http_header* headers = NULL;
  char* status = NULL;
  int h_count = 0;
  char* resource_content = NULL;
  int resource_size = 0;

  printf("IO operation\n");

  switch (req->method)
  {
  case HTTP_GET: {
    // concat paths
    char full_path[MAX_PATH_SIZE] = ROOT_PATH;
    // use url_append here because it works fine in this case
    url_append(full_path, MAX_PATH_SIZE, req->url_path);

    // if root is requested then append index.html
    if (str_is_equal(req->url_path, "/")) {
      url_append(full_path, MAX_PATH_SIZE, "index.html");
    }

    if ((resource_size = read_file(full_path, &resource_content)) < 0) {
      status = "404 Not Found";

      // add item to headers list
      h_count = 1;
      headers = http_create_header_array(h_count,
        (http_header[]) {
          { "Content-Length", "0" },
      });
    }
    else {
      // get file extension
      char extension[8];
      get_file_extension(full_path, extension);

      // get mime type
      char* mime_type =
        records_str_get_value(mime_types, sizeof(mime_types) / sizeof(record_str_t), extension);

      // convert resource_size to string
      char content_size_str[MAX_CONTENT_SIZE_STRLEN];
      int_to_string(resource_size, content_size_str, MAX_CONTENT_SIZE_STRLEN);

      status = "200 OK";

      // add headers
      h_count = 2;
      headers = http_create_header_array(h_count,
        (http_header[]) {
          { "Content-Type", mime_type },
          { "Content-Length", content_size_str },
      });
    }

    break;
  }
  default: {
    h_count = 1;
    headers = http_create_header_array(1,
      (http_header[]) {
      "Content-Length", "0"
    });
    status = "404 Not Found";

    break;
  }
  }

  *res = http_build_response(
    req->variant,
    req->version,
    status,
    headers,
    h_count,
    resource_content,
    &resource_size
  );

  if (headers)
    http_destroy_header_array(headers);
  if (resource_content)
    free_content(resource_content);
}


void on_socket_open(SOCKET socket) {
  printf("connection opened on socket: %d\n", (int)socket);
}
void on_socket_close(SOCKET socket) {
  printf("connection closed on socket: %d\n", (int)socket);
}
void on_server_on() {
  printf("server listening on port: %s...\n\n", DEFAULT_PORT);
}
void on_server_off() {
  printf("server stopped listening\n");
}


int main() {

  http_bind_listener(HTTP_EVENT_CONNECTION_OPEN, on_socket_open);
  http_bind_listener(HTTP_EVENT_CONNECTION_CLOSE, on_socket_close);
  http_bind_listener(HTTP_EVENT_SERVER_ON, on_server_on);
  http_bind_listener(HTTP_EVENT_SERVER_OFF, on_server_off);

  if (!http_init_server(on_request)) {
    error_last_print_message();
  }

  if (!http_server_listen(DEFAULT_HOST, DEFAULT_PORT, 1024)) {
    error_last_print_message();
  }

  return 0;
}