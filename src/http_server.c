#include "http_server.h"

#include "base_server.h"
#include "error.h"

#include <WinSock2.h> // socket library

// edit as needed
#define CONNECTION_RECIEVE_BUFFER_SIZE 10240

// events callback (user defined)
HTTP_REQUEST_CALLBACK _g_onreq = NULL;
HTTP_CONNECTION_CLOSE_CALLBACK _g_onconnclose = NULL;
HTTP_CONNECTION_OPEN_CALLBACK _g_onconnopen = NULL;


CB_RESULT IO_callback(SOCKET client_socket) {
  char buffer[CONNECTION_RECIEVE_BUFFER_SIZE];
  char temp[TEMP_SIZE];

  int ret_val = recv(client_socket, (char*)buffer, CONNECTION_RECIEVE_BUFFER_SIZE, 0);

  if (ret_val < 0) {
    error_set_last_with_code(8, ret_val);
  }
  else if (ret_val == 0) {
    if (_g_onconnclose)
      _g_onconnclose(client_socket);
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
    if (_g_onreq)
      _g_onreq(req, res);

    if (send(client_socket, res, strlen(res), 0) == SOCKET_ERROR) {
      error_set_last_with_code(8, WSAGetLastError());
    }
  }
  return CB_CONTINUE;
}

void http_bind_listener(HTTP_EVENT event, void* callback)
{
  switch (event) {
  case HTTP_EVENT_REQUEST:
    _g_onreq = (HTTP_REQUEST_CALLBACK)callback;
    break;
  case HTTP_EVENT_CONNECTION_OPEN:
    _g_onconnopen = (HTTP_CONNECTION_OPEN_CALLBACK)callback;
    break;
  case HTTP_EVENT_CONNECTION_CLOSE:
    _g_onconnclose = (HTTP_CONNECTION_CLOSE_CALLBACK)callback;
    break;
  default: break;
  }
}

SOCKET create_http_server(const char* const host, const char* const port) {
  if (!init_winsock()) {
    return INVALID_SOCKET;
  }

  SOCKET server_socket = create_listen_socket(host, port, SOCK_STREAM, AF_INET, IPPROTO_TCP);

  if (!server_socket) {
    return INVALID_SOCKET;
  }

  return server_socket;
}

int http_server_listen
(SOCKET server_socket, int max_connections) {
  on_socket_accept(_g_onconnopen);

  if (handle_connections(server_socket, max_connections, IO_callback)) {
    return -1;
  }

  return 0;
}