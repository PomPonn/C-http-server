#pragma once

#include <winsock2.h> // socket programming windows api

#define MemAlloc(flags, size) HeapAlloc(GetProcessHeap(), flags, size)
#define MemFree(pmemory) HeapFree(GetProcessHeap(), 0, pmemory)

typedef int CB_RESULT;

#define CB_CONTINUE 0
#define CB_CLOSE_SOCKET 1

// defines callback function for handle_connections
typedef CB_RESULT(*client_callback)(SOCKET client_socket);

BOOL WINAPI _control_handler(DWORD ctrl_type);

BOOL init_winsock();

SOCKET create_listen_socket(char* host, char* port, int socket_type, int address_family, int protocol);

int handle_connections(SOCKET listen_socket, int max_connections, client_callback callback);