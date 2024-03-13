#pragma once

typedef unsigned int SOCKET;

#define MemAlloc(flags, size) HeapAlloc(GetProcessHeap(), flags, size)
#define MemFree(pmemory) HeapFree(GetProcessHeap(), 0, pmemory)

typedef int CB_RESULT;

#define CB_CONTINUE 0
#define CB_CLOSE_SOCKET 1

// defines callback function for handle_connections
typedef CB_RESULT(*IO_CALLBACK)(SOCKET client_socket);

int init_winsock();

SOCKET create_listen_socket
(
  const char* const host,
  const char* const port,
  int socket_type, int address_family, int protocol
);

/// @brief starts infinite loop to handle client connections
/// @param listen_socket socket to listen on
/// @param max_connections maximum number of simultaneous connections
/// @param on_IO_req callback which runs on client request for I/O operation
/// @return non zero on failure
int handle_connections(SOCKET listen_socket, int max_connections, IO_CALLBACK on_IO_req);