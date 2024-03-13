#pragma once

#include "httpprot.h"

// forward typedefs
typedef int CB_RESULT;
typedef unsigned int SOCKET;

typedef void (*HTTP_REQUEST_CALLBACK)(http_request req, http_response res);

SOCKET create_http_server(const char* const host, const char* const port);

int http_server_listen(SOCKET server_socket, int max_connections);