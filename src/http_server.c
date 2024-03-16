#include "http_server.h"

#include "base_server.h"
#include "error.h"
#include "utils.h"

#include <WinSock2.h> // socket library

// edit as needed
#define CONNECTION_RECIEVE_BUFFER_SIZE 10240

// events callback (user defined)
HTTP_REQUEST_CALLBACK _g_onreq = NULL;
HTTP_CONNECTION_CLOSE_CALLBACK _g_onconnclose = NULL;
HTTP_CONNECTION_OPEN_CALLBACK _g_onconnopen = NULL;
HTTP_SERVER_OFF_CALLBACK _g_onservoff = NULL;
HTTP_SERVER_ON_CALLBACK _g_onservon = NULL;
HTTP_UPGRADE_CALLBACK _g_onupgrade = NULL;

CB_RESULT IO_callback(SOCKET client_socket) {
  char buffer[CONNECTION_RECIEVE_BUFFER_SIZE];
  char temp[TEMP_SIZE];

  int ret_val = recv(client_socket, buffer, CONNECTION_RECIEVE_BUFFER_SIZE, 0);

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

    char* event_header_value = NULL;

    // check for events
    if (_g_onupgrade && http_get_header(req.content, "Upgrade", event_header_value)) {
      // call upgrade callback
      _g_onupgrade(&req, event_header_value, &res);
      free(event_header_value);
    }
    else {
      // call request callback
      if (_g_onreq)
        _g_onreq(&req, &res);
    }

    if (send(client_socket, res, str_length(res), 0) == SOCKET_ERROR) {
      error_set_last_with_code(8, WSAGetLastError());
    }
    if (res)
      http_response_free(res);
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
  case HTTP_EVENT_SERVER_ON:
    _g_onservon = (HTTP_SERVER_ON_CALLBACK)callback;
    break;
  case HTTP_EVENT_SERVER_OFF:
    _g_onservoff = (HTTP_SERVER_OFF_CALLBACK)callback;
    break;
  case HTTP_EVENT_UPGRADE:
    _g_onupgrade = (HTTP_UPGRADE_CALLBACK)callback;
    break;
  default: break;
  }
}

int http_init_server(HTTP_REQUEST_CALLBACK on_req) {
  if (!init_winsock()) {
    return 0;
  }

  on_IO_request(IO_callback);

  if (on_req)
    http_bind_listener(HTTP_EVENT_REQUEST, on_req);

  return 1;
}

int http_server_listen
(char* host, char* port, int max_connections) {
  on_socket_accept(_g_onconnopen);
  on_server_open(_g_onservon);
  on_server_close(_g_onservoff);

  if (create_server(host, port, SOCK_STREAM, AF_INET, IPPROTO_TCP, max_connections)) {
    return 0;
  }

  return 1;
}