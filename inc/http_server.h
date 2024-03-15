#pragma once

#include "httpprot.h"

typedef int HTTP_EVENT;
#define HTTP_EVENT_REQUEST 1
#define HTTP_EVENT_CONNECTION_OPEN 2
#define HTTP_EVENT_CONNECTION_CLOSE 3
#define HTTP_EVENT_UPGRADE 4

// forward typedefs
typedef int CB_RESULT;
typedef unsigned int SOCKET;

typedef void (*HTTP_REQUEST_CALLBACK)(http_request* req, http_response res);
typedef void (*HTTP_CONNECTION_CLOSE_CALLBACK)(SOCKET socket);
typedef void (*HTTP_CONNECTION_OPEN_CALLBACK)(SOCKET socket);
typedef void (*HTTP_UPGRADE_CALLBACK)
(http_request* req, char* upgrade_header_value, http_response res);

/// @brief bind listener to specified event
/// @param event available events: "request", "connection close", "connection open"
/// @param callback callback function to bind. Must be of corresponding callback type.
void http_bind_listener(HTTP_EVENT event, void* callback);

SOCKET create_http_server(const char* const host, const char* const port);

int http_server_listen(SOCKET server_socket, int max_connections);