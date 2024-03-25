#if BASIC

#include "server/http_server.h"

#include "misc/path.h"
#include "misc/utils.h"
#include "structs/pair.h"

#define MAX_PATH_SIZE 256
#define MAX_CONTENT_SIZE_STRLEN 64

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT "8000"

#ifdef _WIN32
#define ROOT_PATH "C:/Users/gangs/OneDrive/Dokumenty/MyStuff/Projects/C_HTTP_server/example/public"
#else
#define ROOT_PATH "/home/filip/Documents/projects/C_server/example/public"
#endif


pair_str_t mime_types[] = {
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

void on_error() {
  error_last_print_message();
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
  printf("\nserver stopped listening\n");
}

int main();

void on_request(http_request* req, http_response* res) {
  http_header* headers = NULL;
  int h_count = 0, h_size = 2;
  char* status = NULL;
  char* resource_content = NULL;
  int resource_size = 0;

  headers = http_header_array_create(2, 0, NULL);

  switch (req->method)
  {
  case HTTP_GET: {
    // concat paths
    char full_path[MAX_PATH_SIZE] = ROOT_PATH;
    path_join(full_path, MAX_PATH_SIZE, req->url_path);

    // if root is requested then append index.html
    if (str_is_equal(req->url_path, "/")) {
      path_join(full_path, MAX_PATH_SIZE, "index.html");
    }

    if ((resource_size = read_file(full_path, &resource_content)) < 0) {
      status = "404 Not Found";

      // add item to headers list
      h_count = http_header_array_push(h_size, h_count, headers, 2,
        (http_header[]) {
          { "Content-Length", "0" },
          { "Connection", "close" },
      });
    }
    else {
      // get file extension
      char extension[8];
      get_file_extension(full_path, extension);

      // get mime type
      char* mime_type =
        pairs_str_get_value(mime_types, sizeof(mime_types) / sizeof(pair_str_t), extension);

      // convert resource_size to string
      char content_size_str[MAX_CONTENT_SIZE_STRLEN];
      int_to_string(resource_size, content_size_str, MAX_CONTENT_SIZE_STRLEN);

      status = "200 OK";

      // add headers
      h_count = http_header_array_push(h_size, h_count, headers, 2,
        (http_header[]) {
          { "Content-Type", mime_type },
          { "Content-Length", content_size_str },
          { "Connection", "keep-alive" },
      });
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
      http_header_array_destroy(headers);

    if (resource_content)
      free_content(resource_content);
    break;
  }
  }
}

int main() {

  error_set_callback((ERROR_CALLBACK)on_error);
  http_bind_listener(HTTP_EVENT_CONNECTION_OPEN, on_socket_open);
  http_bind_listener(HTTP_EVENT_CONNECTION_CLOSE, on_socket_close);
  http_bind_listener(HTTP_EVENT_SERVER_ON, on_server_on);
  http_bind_listener(HTTP_EVENT_SERVER_OFF, on_server_off);

  SOCKET server_socket = http_create_server(DEFAULT_HOST, DEFAULT_PORT);

  return http_server_listen(server_socket, 1024, on_request);
}

#endif