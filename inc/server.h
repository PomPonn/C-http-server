#pragma once

#include <winsock2.h>

#define INVALID_SERVER INVALID_SOCKET

typedef SOCKET server_t;

// buffer is guaranteed to be unmutable
char* get_buffer_line(char* buffer, char** last_char);

server_t create_server(char* port);

DWORD WINAPI handle_client(void* args);

int listen_on(server_t server, void (*response)(SOCKET client_socket));