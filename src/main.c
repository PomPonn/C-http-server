#include "httpprot.h"

#include "server.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

#define FILE_PATH "C:/Users/gangs/OneDrive/Dokumenty/MyStuff/Projects/C_HTTP_server/index.html"
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
    printf("connected: %d\n", client_socket);

    // get request method

    char* ptr = buffer;
    char line[LINE_LEN];
    if (!(ptr = get_buffer_line(buffer, line, LINE_LEN))) {
      printf("http parsing error\n");
      return CB_CONTINUE;
    }

    http_request req;
    if (resolve_http_request(line, &req)) {
      printf("http parsing error\n");
      return CB_CONTINUE;
    }

    // handle request
    switch (req.method) {
    case HTTP_GET: {

      if (strcmp(req.path, "/") == 0) {
        http_header h_accept;
        h_accept.name = "Accept";

        if (!find_http_header_value(ptr, &h_accept)) {
          printf("http parsing error\n");
          return CB_CONTINUE;
        }

        if (!is_in_header_value(h_accept.value, ',', "text/html")) {
          printf("http parsing error\n");
          return CB_CONTINUE;
        }

        http_header_free(&h_accept);

        // open index.html file
        FILE* fp;
        fopen_s(&fp, FILE_PATH, "rb");

        if (!fp) {
          printf("failed to open resource file: %s\n", FILE_PATH);
          return CB_CONTINUE;
        }

        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* file_buf = malloc(file_size + 1);

        if (fread(file_buf, sizeof(char), file_size, fp) != file_size) {
          printf("Error while reading resource file: %s\n", FILE_PATH);
          return CB_CONTINUE;
        }
        file_buf[file_size] = '\0';

        _itoa_s(file_size, line, LINE_LEN, 10);

        http_header resp_headers[] = {
          { "Content-Length", line },
          { "Content-Type", "text/html; charset=utf-8" },
        };
        int response_headers_count = sizeof(resp_headers) / sizeof(http_header);

        char* response = build_http_response(req.version, "200 OK",
          resp_headers, response_headers_count, file_buf);

        free(file_buf);
        fclose(fp);

        send(client_socket, response, strlen(response), 0);

        free(response);
      }
      else {
        send(client_socket, "HTTP/1.1 404\r\nContent-Length: 0\r\n", 34, 0);
      }
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