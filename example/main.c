#include "http_server.h"
#include "url.h"
#include "utils.h"
#include "structs/record.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_PATH_SIZE 256
#define MAX_CONTENT_SIZE_STRLEN 64

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
  http_header_list_item* headers;

  switch (req->method)
  {
  case HTTP_GET: {
    char* status;
    int h_count = 0;
    char* resource_content = NULL;
    int resource_size = 0;

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
      h_count++;
      headers = http_create_header_list(1,
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
      _itoa_s(resource_size, content_size_str, MAX_CONTENT_SIZE_STRLEN, 10);

      status = "200 OK";

      // add headers
      headers = http_create_header_list(2,
        (http_header[]) {
          { "Content-Type", mime_type },
          { "Content-Length", content_size_str },
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

    http_destroy_header_list(headers);
    free_content(resource_content);
    break;
  }
  default: {
    headers = http_create_header_list(1,
      (http_header[]) {
      "Content-Length", "0"
    });

    *res = http_build_response(
      req->variant,
      req->version,
      "404 Not Found",
      headers,
      1, NULL, NULL);
    break;
  }
  }
}

void on_socket_close(SOCKET socket) {
  printf("connection closed on socket: %d\n", (int)socket);
}

void on_socket_open(SOCKET socket) {
  printf("connection opened on socket: %d\n", (int)socket);
}

int main() {
  http_bind_listener(HTTP_EVENT_CONNECTION_OPEN, on_socket_open);
  http_bind_listener(HTTP_EVENT_CONNECTION_CLOSE, on_socket_close);

  SOCKET server = create_http_server("localhost", "80", on_request);

  http_server_listen(server, 128);

  return 0;
}