#include "http_server.h"
#include "url.h"

#include <stdio.h>
#include <string.h>

#define MAX_PATH_SIZE 256

#define INDEX_PATH "C:/Users/gangs/OneDrive/Dokumenty/MyStuff/Projects/C_HTTP_server/"

void on_connection(http_request req, http_response res) {
  // concat paths
  char full_path[MAX_PATH_SIZE] = INDEX_PATH;
  url_append(full_path, MAX_PATH_SIZE, req.url_path);

  // if root is requested then get index.html
  if (strcmp(req.url_path, "/") == 0) {
    url_append(full_path, MAX_PATH_SIZE, "index.html");
  }

  // get file extension
  char extension[8];
  get_file_extension(full_path, extension);

  url_parts url;
  url_parse("https://poczta.wp.pl/login/login.html#/mails/?label=1", &url);
  /*
  // handle request
  switch (req.method) {
  case HTTP_GET: {
    http_header h_accept;
    h_accept.name = "Accept";

        if (!get_http_header_value(ptr, &h_accept)) {
          printf("http parsing error\n");
          return CB_CONTINUE;
        }

      // free accept header, since it wont be needed anymore
      http_header_free(&h_accept);

      char* file_buffer = NULL;
      // get requested resource
      int file_size = get_resource(full_path, &file_buffer);

      if (file_size < 0) {
        printf("Error while reading requested http resource: %s\n", full_path);
        send(client_socket, "HTTP/1.1 404\r\nContent-Length: 0\r\n", 34, 0);
        return;
      }

      // convert file size to string
      char length_str[16];
      _itoa_s(file_size, length_str, 16, 10);

      // make content-type value
      char content_type[32] = "text/"; // to change
      strcat_s(content_type, 32, extension);
      strcat_s(content_type, 32, "; charset=utf-8");

      // make response headers
      http_header resp_headers[] = {
        { "Content-Length", length_str },
        { "Content-Type", content_type },
      };

      int response_headers_count = sizeof(resp_headers) / sizeof(http_header);
      int response_size = 0;

      char* response = build_http_response(req.version, "200 OK",
        resp_headers, response_headers_count, file_buffer, &response_size);

      free_resource(file_buffer);
      if (response_size < 0) {
        printf("Failed to build http response: %d\n", response_size);
        send(client_socket, "HTTP/1.1 404 Bad Request\r\nContent-Length: 0\r\n", 46, 0);
        return;
      }

      send(client_socket, response, response_size, 0);

      free_http_response(response);
      break;
    }
    default: {
      send(client_socket, "HTTP/1.1 404\r\nContent-Length: 0\r\n", 34, 0);
      break;
    }*/
}

int main() {

  SOCKET server = create_http_server("localhost", "80");

  http_server_listen(server, 128, on_connection);

  return 0;
}