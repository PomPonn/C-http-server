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

void on_IO_request(IO_CALLBACK callback);

void on_socket_accept(SOCK_ACCEPT_CALLBACK callback);

void on_server_open(SERVER_OPEN_CALLBACK callback);

void on_server_close(SERVER_CLOSE_CALLBACK callback);

SOCKET create_server_socket
(const char* host, const char* port, int server_socket_type, int protocol);

int start_listening(SOCKET server_socket, int max_connections);