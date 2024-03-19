#pragma once

#include "misc/utils.h"
#include "misc/error.h"
#include "httpprot.h"

// http events
typedef int HTTP_EVENT;
#define HTTP_EVENT_REQUEST 1
#define HTTP_EVENT_CONNECTION_OPEN 2
#define HTTP_EVENT_CONNECTION_CLOSE 3
#define HTTP_EVENT_SERVER_ON 4
#define HTTP_EVENT_SERVER_OFF 5

// forward typedefs
typedef int CB_RESULT;
#ifdef _WIN32
typedef unsigned int SOCKET;
#elif __linux__
typedef int SOCKET;
#endif

// http callbacks
typedef void (*HTTP_REQUEST_CALLBACK)(http_request* req, http_response* res);
typedef void (*HTTP_CONNECTION_CLOSE_CALLBACK)(SOCKET socket);
typedef void (*HTTP_CONNECTION_OPEN_CALLBACK)(SOCKET socket);
typedef void (*HTTP_SERVER_ON_CALLBACK)(void);
typedef void (*HTTP_SERVER_OFF_CALLBACK)(void);


/// @brief bind listener to specified event
/// @param event HTTP...CALLBACK
/// @param callback function to bind. Must be of corresponding callback type.
void http_bind_listener(HTTP_EVENT event, void* callback);

/// @brief creates new server socket
/// @param host host to bind new socket to
/// @param port port of the host to bind new socket to
/// @returns new server socket
SOCKET http_create_server
(const char* host, const char* port);

/// @brief starts infinite loop which handles client connections
/// @param server_socket socket to listen on
/// @param max_connections maximum number of simultaneous connections
/// @param on_request optional callback to call on request (can also be set with http_bind_listener)
/// @return zero on success, nonzero on failure
int http_server_listen
(SOCKET server_socket, int max_connections, HTTP_REQUEST_CALLBACK on_request);