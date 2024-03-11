#include "httpprot.h"

#include "server.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

// cannot end with a slash
#define INDEX_PATH "C:/Users/gangs/OneDrive/Dokumenty/MyStuff/Projects/C_HTTP_server"

#define MAX_PATH_SIZE 128
#define LINE_LEN 128

CB_RESULT callback(SOCKET client_socket) {
  char buffer[1024];

  int ret_val = recv(client_socket, (char*)buffer, sizeof(buffer), 0);

  if (ret_val < 0) {
    printf("recv error: %d\n", ret_val);
  }
  else if (ret_val == 0) {
    printf("disconnected: %d\n", client_socket);
    return CB_CLOSE_SOCKET;
  }
  else {
    char* ptr = buffer;
    char line[LINE_LEN];
    http_request_line req;

    // read buffer line;
    if (!(ptr = get_buffer_line(buffer, line, LINE_LEN))) {
      printf("http parsing error\n");
      return CB_CONTINUE;
    }

    if (resolve_http_request_line(line, &req)) {
      printf("http parsing error\n");
      return CB_CONTINUE;
    }

    // if root is requested then get index.html
    if (req.path == "/") {
      strcat_s(req.path, MAX_PATH_SIZE, "index.html");
    }

    // concat paths
    char full_path[MAX_PATH_SIZE] = INDEX_PATH;
    strcat_s(full_path, MAX_PATH_SIZE, req.path);

    // handle request
    switch (req.method) {
    case HTTP_GET: {
      http_header h_accept;
      h_accept.name = "Accept";

      if (!get_http_header_value(ptr, &h_accept)) {
        printf("http parsing error\n");
        return CB_CONTINUE;
      }

      // get file extension
      char extension[8];
      get_file_extension(full_path, extension);

      // switch on the file extension
      if (strcmp(extension, "html") == 0) {
        if (!is_in_http_header(&h_accept, ',', "text/html")) {
          printf("http parsing error\n");
          return CB_CONTINUE;
        }
      }
      else {
        printf("http parsing error: unsupported file extension\n");
        return CB_CONTINUE;
      }

      // free accept header, since it wont be needed anymore
      http_header_free(&h_accept);

      char* file_buffer = NULL;
      // get requested resource
      int file_size = get_resource(full_path, file_buffer);

      // convert file size to string
      char buff[16];
      _itoa_s(file_size, buff, 16, 10);

      // make response headers
      http_header resp_headers[] = {
        { "Content-Length", buff },
        { "Content-Type", "text/html; charset=utf-8" },
      };

      int response_headers_count = sizeof(resp_headers) / sizeof(http_header);
      int response_size = 0;

      char* response = build_http_response(req.version, "200 OK",
        resp_headers, response_headers_count, file_buffer, &response_size);

      free_resource(file_buffer);
      if (response_size < 0) {
        return CB_CONTINUE;
      }

      send(client_socket, response, response_size, 0);

      free_http_response(response);
      break;
    }
    default: {
      send(client_socket, "HTTP/1.1 404\r\nContent-Length: 0\r\n", 34, 0);
    }
    }
  }
  return CB_CONTINUE;
}

int main() {
  init_winsock();

  SOCKET listener = create_listen_socket("localhost", "80", SOCK_STREAM, AF_INET, IPPROTO_TCP);

  handle_connections(listener, 100, callback);

  return 0;
}