#include "base_server.h"

#include <WinSock2.h> // socket library
#include <ws2tcpip.h> // address resolve
#include <stdio.h>    // console output

// inform compiler to use winsock library
#pragma comment(lib, "Ws2_32.lib")

// GLOBAL
int* connections;
// for _control_handler to access maximum number of connections
int g_max_conns;
BOOL g_quit = FALSE;

BOOL WINAPI _control_handler(DWORD ctrl_type);

int handle_connections(SOCKET listen_socket, int max_connections, connection_callback callback) {
  // verify parameters
  if (max_connections < 1 || listen_socket == INVALID_SOCKET || callback == NULL)
    return -1;

  fd_set fd_read_set;
  int ret_val = 0;
  g_max_conns = max_connections;

  // allocate memory for client sockets (+1 server socket)
  connections = MemAlloc(0, sizeof(SOCKET) * (max_connections + 1));

  // init connections and let the first one be the server socket
  connections[0] = listen_socket;
  for (int i = 1; i <= max_connections; i++) {
    connections[i] = -1;
  }

  // set control handler to make cleanup after server shutdown
  SetConsoleCtrlHandler(_control_handler, TRUE);

  // start handling connections
  while (!g_quit) {
    // set fd_read_set before select call
    FD_ZERO(&fd_read_set);
    for (int i = 0; i <= max_connections; i++) {
      if (connections[i] >= 0)
        FD_SET(connections[i], &fd_read_set);
    }

    // wait until select return
    ret_val = select(0, &fd_read_set, NULL, NULL, NULL);

    if (ret_val > 0) {
      // check if socket with event is listen_socket
      if (FD_ISSET(listen_socket, &fd_read_set)) {
        // accept new client connection
        SOCKET client_socket = accept(listen_socket, NULL, NULL);

        if (client_socket == INVALID_SOCKET && !g_quit) {
          printf("Accept failed: %d\n", WSAGetLastError());
        }
        else {
          // add new client socket to the first free connections slot
          for (int i = 1; i <= max_connections; i++) {
            if (connections[i] < 0) {
              connections[i] = client_socket;
              break;
            }
          }
        }

        if (--ret_val == 0)
          continue;
      }

      // iterate connections in search of sockets with event
      for (int i = 1; i <= max_connections; i++) {
        if (connections[i] > 0 && FD_ISSET(connections[i], &fd_read_set)) {

          // run user defined callback function
          CB_RESULT res = callback(connections[i]);

          // react to callback result
          switch (res) {
          case CB_CONTINUE:
            break;
          case CB_CLOSE_SOCKET:
            connections[i] = -1; // invalidate connection
            break;
          }

          // end loop if all connections have been found
          if (--ret_val == 0)
            break;
        }
      }
    }
    else {
      if (!g_quit)
        printf("Select failed: %d\n", WSAGetLastError());
    }
  } // while (TRUE)

  return 0;
}

BOOL WINAPI _control_handler(DWORD ctrl_type) {
  switch (ctrl_type)
  {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
  {
    g_quit = TRUE;
    for (int i = 0; i < g_max_conns; i++) {
      if (connections[i] > 0) {
        closesocket(connections[i]);
      }
    }
    MemFree(connections);

    WSACleanup();

    ExitProcess(0);
    return TRUE;
  }
  default:
    return FALSE;
  }
}

BOOL init_winsock() {
  WSADATA wsa_data;

  // initialize winsock dll
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    printf("WSAStartup failed\n");
    return 0;
  }

  return 1;
}

SOCKET create_listen_socket
(
  const char* const host,
  const char* const port,
  int socket_type, int address_family, int protocol
) {
  struct addrinfo* result, * ptr, hints;

  // init hints structure
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = address_family;
  hints.ai_socktype = socket_type;
  hints.ai_protocol = protocol;
  hints.ai_flags = AI_PASSIVE;

  // get address info
  int err_code = getaddrinfo(host, port, &hints, &result);
  if (err_code) {
    printf("getaddrinfo failed with error code %d\n", err_code);
    WSACleanup();
    return INVALID_SOCKET;
  }

  SOCKET listen_socket;
  ptr = result;

  while (ptr->ai_addr != NULL) {
    // create socket
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket != INVALID_SOCKET) {
      // bind socket
      err_code = bind(listen_socket, result->ai_addr, result->ai_addrlen);

      if (!err_code)
        break;

      listen_socket = INVALID_SOCKET;
      ptr = ptr->ai_next;
    }

  }

  if (listen_socket == INVALID_SOCKET) {
    printf("Error at socket creation: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return INVALID_SOCKET;
  }

  freeaddrinfo(result);

  // listen for connections
  if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
    printf("Listen failed with error: %d\n", WSAGetLastError());
    closesocket(listen_socket);
    WSACleanup();
    return INVALID_SOCKET;
  }

  return listen_socket;
}