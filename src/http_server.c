#include "http_server.h"

#include "base_server.h"
#include "error.h"

#include <WinSock2.h> // socket library
#include <stdio.h>

#define CONNECTION_RECIEVE_BUFFER_SIZE 10240

// events callback (user defined)
HTTP_REQUEST_CALLBACK _g_onreq = NULL;

// update here
CB_RESULT IO_callback(SOCKET client_socket) {
  char buffer[CONNECTION_RECIEVE_BUFFER_SIZE];
  char temp[8];

  int ret_val = recv(client_socket, (char*)buffer, CONNECTION_RECIEVE_BUFFER_SIZE, 0);

  if (ret_val < 0) {
    error_set_last(8, itoa(ret_val, temp, 10));
  }
  else if (ret_val == 0) {
    // event here?
    closesocket(client_socket);
    return CB_CLOSE_SOCKET;
  }
  else {
    http_request req;
    http_response res = NULL;

    // fill req struct
    if (resolve_http_request_line(buffer, &req)) {
      error_set_last(9, "IO_callback");
      return CB_CONTINUE;
    }

    // call callback
    // update this
    _g_onconn(req, res);

    if (send(client_socket, res, strlen(res), 0) == SOCKET_ERROR) {
      error_set_last(10, itoa(WSAGetLastError(), temp, 10));
    }
  }
  return CB_CONTINUE;
}

SOCKET create_http_server(const char* const host, const char* const port) {
  if (!init_winsock()) {
    error_last_print_message();
    return INVALID_SOCKET;
  }

  SOCKET server_socket = create_listen_socket(host, port, SOCK_STREAM, AF_INET, IPPROTO_TCP);

  if (!server_socket) {
    error_last_print_message();
    return INVALID_SOCKET;
  }

  return server_socket;
}

int http_server_listen
(SOCKET server_socket, int max_connections) {
  if (handle_connections(server_socket, max_connections, IO_callback)) {
    error_last_print_message();
    return -1;
  }

  return 0;
}

