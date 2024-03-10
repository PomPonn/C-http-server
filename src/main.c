#include "httpprot.h"

#include "server.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

// cant end with '/'
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
    // get request method

    char* ptr = buffer;
    char line[LINE_LEN];
    if (!(ptr = get_buffer_line(buffer, line, LINE_LEN))) {
      printf("http parsing error\n");
      return CB_CONTINUE;
    }

    http_request_line req;
    if (resolve_http_request_line(line, &req)) {
      printf("http parsing error\n");
      return CB_CONTINUE;
    }

    char full_path[MAX_PATH_SIZE] = INDEX_PATH;
    strcat_s(full_path, MAX_PATH_SIZE, req.path);

    // handle request
    switch (req.method) {
    case HTTP_GET: {
      char* response = NULL;
      int res_size = get_resource(full_path, MAX_PATH_SIZE, req.version, ptr, response);
      if (res_size < 0) {
        return CB_CONTINUE;
      }

      send(client_socket, response, res_size, 0);
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