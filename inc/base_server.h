#pragma once

typedef unsigned int SOCKET;

#define MemAlloc(flags, size) HeapAlloc(GetProcessHeap(), flags, size)
#define MemFree(pmemory) HeapFree(GetProcessHeap(), 0, pmemory)

typedef int CB_RESULT;

#define CB_CONTINUE 0
#define CB_CLOSE_SOCKET 1

typedef CB_RESULT(*IO_CALLBACK)(SOCKET client_socket);
typedef void(*SOCK_ACCEPT_CALLBACK)(SOCKET client_socket);
typedef void(*SERVER_CLOSE_CALLBACK)(void);
typedef void(*SERVER_OPEN_CALLBACK)(void);

int init_winsock();

SOCKET create_listen_socket
(
  const char* const host,
  const char* const port,
  int socket_type, int address_family, int protocol
);

void on_IO_request(IO_CALLBACK callback);

void on_socket_accept(SOCK_ACCEPT_CALLBACK callback);

void on_server_open(SERVER_OPEN_CALLBACK callback);

void on_server_close(SERVER_CLOSE_CALLBACK callback);

/// @brief starts infinite loop to handle client connections
/// @param listen_socket socket to listen on
/// @param max_connections maximum number of simultaneous connections
/// @param on_IO_req callback which runs on client request for I/O operation
/// @return non zero on failure
int handle_connections(SOCKET listen_socket, int max_connections, IO_CALLBACK on_IO_req);