#pragma once

#ifdef _WIN32

#define IP_PROT_TCP IPPROTO_TCP

#define default_last_err WSAGetLastError()
#define socket_close(socket) closesocket(socket)
#define cleanup() cleanup()

#elif __linux__

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define FALSE 0
#define TRUE 1

#define socket_close(socket) close(socket)
#define default_last_err errno
#define cleanup() 
#define SD_BOTH SHUT_RDWR
#define IP_PROT_TCP 6

typedef int BOOL;

#endif