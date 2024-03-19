#pragma once

#include "misc/error.h"

// possible request callback results
typedef int CB_RESULT;
#define CB_CONTINUE 0
#define CB_CLOSE_SOCKET 1

// forward typedefs
typedef unsigned int SOCKET;

// callback types
typedef CB_RESULT(*IO_CALLBACK)(SOCKET client_socket);
typedef void(*SOCK_ACCEPT_CALLBACK)(SOCKET client_socket);
typedef void(*SERVER_CLOSE_CALLBACK)(void);
typedef void(*SERVER_OPEN_CALLBACK)(void);

#define MemAlloc(flags, size) HeapAlloc(GetProcessHeap(), flags, size)
#define MemFree(pmemory) HeapFree(GetProcessHeap(), 0, pmemory)


/// @brief initialize WinSock library
/// @return 1 on success, 0 on failure
int init_winsock();

void on_IO_request(IO_CALLBACK callback);

void on_socket_accept(SOCK_ACCEPT_CALLBACK callback);

void on_server_open(SERVER_OPEN_CALLBACK callback);

void on_server_close(SERVER_CLOSE_CALLBACK callback);

/// @brief creates server socket on provided host and port
/// @param server_socket_type type of the socket (win api)
/// @param protocol protocol to use (win api)
/// @returns created socket
SOCKET create_server_socket
(const char* host, const char* port, int server_socket_type, int protocol);

/// @brief starts to handle client connections
/// @param server_socket server socket to listen on
/// @param max_connections mamimum number of simultaneous client connections
/// @return 0 on success, nonzero on failure
int start_listening(SOCKET server_socket, int max_connections);