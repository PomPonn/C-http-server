#include "http_server.h"

#include "base_server.h"

#include <WinSock2.h> // socket library
#include <stdio.h>

#define CONNECTION_RECIEVE_BUFFER_SIZE 10240

// GLOBAL
HTTP_CONNECTION_CALLBACK _g_onconn;

CB_RESULT http_connection_callback_wrapper(SOCKET client_socket) {
  char buffer[CONNECTION_RECIEVE_BUFFER_SIZE];

  int ret_val = recv(client_socket, (char*)buffer, CONNECTION_RECIEVE_BUFFER_SIZE, 0);

  if (ret_val < 0) {
    printf("recv error: %d\n", ret_val);
  }
  else if (ret_val == 0) {
    closesocket(client_socket);
    return CB_CLOSE_SOCKET;
  }
  else {
    http_request req;
    http_response res = NULL;

    // fill req struct
    if (resolve_http_request_line(buffer, &req)) {
      printf("http parsing error\n");
      return CB_CONTINUE;
    }

    // call callback
    _g_onconn(req, res);

    send(client_socket, res, strlen(res), 0);
  }
  return CB_CONTINUE;
}

SOCKET create_http_server(const char* const host, const char* const port) {
  if (!init_winsock())
    return INVALID_SOCKET;

  SOCKET server_socket = create_listen_socket(host, port, SOCK_STREAM, AF_INET, IPPROTO_TCP);

  return server_socket;
}

int http_server_listen
(SOCKET server_socket, int max_connections, HTTP_CONNECTION_CALLBACK on_connection) {
  _g_onconn = on_connection;

  if (handle_connections(server_socket, max_connections, http_connection_callback_wrapper)) {
    return -1;
  }

  return 0;
}

