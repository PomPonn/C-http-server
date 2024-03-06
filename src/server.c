#include "server.h"

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdio.h>

// inform compiler to use winsock library
#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 512


char* get_buffer_line(char* buffer, char** last_char) {
  char* line, * ptr = buffer;
  int line_size = 0;

  // find size line length ( check for \r\n string [CRLF])
  while (*ptr != '\r' || *(ptr + 1) != '\n') {
    ptr++;
    line_size++;
  }

  line = malloc(line_size);
  ptr = buffer;

  for (int i = 0; i < line_size; i++) {
    line[i] = *ptr;
    ptr++;
  }
  line[line_size] = '\0';

  *last_char = ptr + 2;

  return line;
}

DWORD WINAPI handle_client(void* args) {
  SOCKET client_socket = *(SOCKET*)args;

  char data_buff[BUFLEN];
  int recv_result, send_result;

  do {
    recv_result = recv(client_socket, data_buff, BUFLEN, 0);
    if (recv_result > 0) {
      // handle response
      char* ptr = NULL;
      char* line = get_buffer_line(data_buff, &ptr);

      // skip if request is not HTTP
      // extend
      if (strcmp(line, "GET / HTTP/1.1")) {
        return 0;
      }

      send_result = send(client_socket, data_buff, BUFLEN, 0);

      if (send_result == SOCKET_ERROR) {
        printf("[SERVER ERROR]\tsend failed: %d\n", WSAGetLastError());
        closesocket(send_result);
        WSACleanup();
        return 1;
      }
    }
    else if (recv_result == 0) {
      printf("[SERVER LOG]\tConnection closing...\n");
    }
    else {
      printf("[SERVER ERROR]\trecv failed: %d\n", WSAGetLastError());
      closesocket(client_socket);
      WSACleanup();
      return 1;
    }
  } while (recv_result > 0);

  // shutdown the connection
  int err_code = shutdown(client_socket, SD_BOTH);
  if (err_code == SOCKET_ERROR) {
    printf("[SERVER ERROR]\tShutdown failed: %d\n", WSAGetLastError());
    closesocket(client_socket);
    WSACleanup();
    return 1;
  }

  // cleanup
  closesocket(client_socket);

  return 0;
}

server_t create_server(char* port) {
  WSADATA wsa_data;

  // initialize winsock dll
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    printf("[SERVER ERROR]\tWSAStartup failed\n");
    return 1;
  }

  struct addrinfo* result, hints;

  // init hints structure
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // stream socket
  hints.ai_protocol = IPPROTO_TCP; // TCP protocol
  hints.ai_flags = AI_PASSIVE;

  // get address info
  int err_code = getaddrinfo(NULL, port, &hints, &result);
  if (err_code) {
    printf("[SERVER ERROR]\tgetaddrinfo failed with error code %d\n", err_code);
    WSACleanup();
    return INVALID_SERVER;
  }

  // create socket
  SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (listen_socket == INVALID_SOCKET) {
    printf("[SERVER ERROR]\tError at socket %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return INVALID_SERVER;
  }

  // bind socket
  err_code = bind(listen_socket, result->ai_addr, result->ai_addrlen);
  if (err_code) {
    printf("[SERVER ERROR]\tError at socket binding: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(listen_socket);
    WSACleanup();
    return INVALID_SOCKET;
  }

  freeaddrinfo(result);

  return listen_socket;
}

int listen_on(server_t server, void (*response)(SOCKET client_socket)) {
  if (listen(server, SOMAXCONN) == SOCKET_ERROR) {
    printf("[SERVER ERROR]\tListen failed with error: %d\n", WSAGetLastError());
    closesocket(server);
    WSACleanup();
    return INVALID_SOCKET;
  }

  SOCKET client_socket;

  while (1) {
    // accept connection on a socket
    client_socket = accept(server, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
      printf("accept failed: %d\n", WSAGetLastError());
      closesocket(client_socket);
      WSACleanup();
      return INVALID_SOCKET;
    }

    // pass connection to another thread
    CreateThread(NULL, 0, handle_client, (void*)&client_socket, 0, NULL);
  }

  // ???
  WSACleanup();

  return 0;
}