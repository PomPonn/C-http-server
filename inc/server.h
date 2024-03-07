#pragma once

#include <winsock2.h>

// buffer is guaranteed to be unmutable
char* _get_buffer_line(char* buffer, char** last_char);

SOCKET create_tcp_server_socket(char* port);

VOID CALLBACK _handle_client_wrapper(PTP_CALLBACK_INSTANCE instance, PVOID param, PTP_WORK work);

void close_client_socket(SOCKET client_socket);

int handle_connections(SOCKET server_socket, int max_connections, void (*handle_client)(SOCKET client_socket));