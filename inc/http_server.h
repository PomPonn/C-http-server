#pragma once

#include "httpprot.h"

typedef int HTTP_EVENT;
#define HTTP_EVENT_REQUEST 1
#define HTTP_EVENT_CONNECTION_OPEN 2
#define HTTP_EVENT_CONNECTION_CLOSE 3
#define HTTP_EVENT_UPGRADE 4
#define HTTP_EVENT_SERVER_ON 5
#define HTTP_EVENT_SERVER_OFF 6
// forward typedefs
typedef int CB_RESULT;
typedef unsigned int SOCKET;

typedef void (*HTTP_REQUEST_CALLBACK)(http_request* req, http_response* res);
typedef void (*HTTP_CONNECTION_CLOSE_CALLBACK)(SOCKET socket);
typedef void (*HTTP_CONNECTION_OPEN_CALLBACK)(SOCKET socket);
typedef void (*HTTP_SERVER_ON_CALLBACK)(void);
typedef void (*HTTP_SERVER_OFF_CALLBACK)(void);

typedef void (*HTTP_UPGRADE_CALLBACK)
(http_request* req, char* upgrade_header_value, http_response* res);

typedef struct MIME_record {
  char* extension;
  char* type;
} MIME_record;

/// @brief bind listener to specified event
/// @param event available events: "request", "connection close", "connection open"
/// @param callback callback function to bind. Must be of corresponding callback type.
void http_bind_listener(HTTP_EVENT event, void* callback);

int http_init_server(HTTP_REQUEST_CALLBACK on_request);

int http_server_listen(char* host, char* port, int max_connections);