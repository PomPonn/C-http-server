#pragma once

#include <winsock2.h> // socket networking

#define MemAlloc(flags, size) HeapAlloc(GetProcessHeap(), flags, size)
#define MemFree(pmemory) HeapFree(GetProcessHeap(), 0, pmemory)

typedef int CB_RESULT;

#define CB_CONTINUE 0
#define CB_CLOSE_SOCKET 1

typedef CB_RESULT(*client_callback)(SOCKET client_socket);

// use to access the callback parameters passed to callback wrapper through PVOID param
#define _CB_PARAM(callback_args, param_type, mem_offset) (*(param_type*)((BYTE*)callback_args + sizeof(client_callback) + mem_offset))

// use to access the callback pointer passed to callback wrapper through PVOID param
#define _CB_CALLBACK(callback_args) (*(client_callback*)callback_args)


SOCKET create_tcp_server_socket(char* port);

VOID CALLBACK _callback_wrapper(PTP_CALLBACK_INSTANCE instance, PVOID param, PTP_WORK work);

int handle_connections(SOCKET server_socket, int max_connections, client_callback callback);