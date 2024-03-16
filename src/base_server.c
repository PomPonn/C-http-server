#include "base_server.h"

#include "error.h"

#define WINDOWS_LEAN_AND_MEAN
#define FD_SETSIZE 1024

#include <WinSock2.h> // socket library
#include <ws2tcpip.h> // address resolve

// inform compiler to use winsock library
#pragma comment(lib, "Ws2_32.lib")

/* GLOBAL */
SOCKET* connections;
// for _control_handler to access maximum number of connections
int g_max_conns;
BOOL g_quit = FALSE;

IO_CALLBACK g_io_cb = NULL;
SOCK_ACCEPT_CALLBACK g_sock_acc_cb = NULL;
SERVER_CLOSE_CALLBACK g_serv_close_cb = NULL;
SERVER_OPEN_CALLBACK g_serv_open_cb = NULL;

BOOL WINAPI _control_handler(DWORD ctrl_type);

int create_server
(const char* host, const char* port, int server_socket_type, int address_family, int protocol, int max_connections)
{
  if (!port || max_connections < 1) {
    error_set_last(1, "create_server");
    WSACleanup();
    return -1;
  }
  g_max_conns = max_connections;

  //
  // Create server socket
  //
  SOCKET server_socket = INVALID_SOCKET;
  struct addrinfo hints, * result = NULL;
  char temp[8];
  u_long io_mode = 1;

  // set hints struct
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = address_family;
  hints.ai_protocol = protocol;
  hints.ai_socktype = server_socket_type;
  hints.ai_flags = AI_PASSIVE;

  int retval = getaddrinfo(host, port, &hints, &result);

  if (retval || !result) {
    error_set_last_with_code(5, WSAGetLastError());
    WSACleanup();
    return -1;
  }

  // Create server socket
  server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (server_socket == INVALID_SOCKET)
  {
    error_set_last_with_code(6, WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return -1;
  }

  // Bind the socket
  retval = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
  if (retval == SOCKET_ERROR)
  {
    error_set_last_with_code(11, WSAGetLastError());
    freeaddrinfo(result);
    closesocket(server_socket);
    WSACleanup();
    return -1;
  }

  freeaddrinfo(result);

  // Make the socket non-blocking
  retval = ioctlsocket(server_socket, FIONBIO, &io_mode);
  if (retval == SOCKET_ERROR)
  {
    error_set_last_with_code(12, WSAGetLastError());
    closesocket(server_socket);
    WSACleanup();
    return -1;
  }

  // listen for connections
  if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
    error_set_last_with_code(7, WSAGetLastError());
    closesocket(server_socket);
    WSACleanup();
    return -1;
  }

  // set control handler to make cleanup after server shutdown
  SetConsoleCtrlHandler(_control_handler, TRUE);

  //
  // Start connections loop
  //
  fd_set fd_read_set;
  int ret_val = 0;
  g_max_conns = max_connections;

  // allocate memory for client sockets (+1 for server socket)
  connections = MemAlloc(0, sizeof(SOCKET) * (max_connections + 1));

  // init connections and let the first one be the server socket
  connections[0] = server_socket;
  for (int i = 1; i <= max_connections; i++) {
    connections[i] = INVALID_SOCKET;
  }

  if (g_serv_open_cb)
    g_serv_open_cb();

  // start handling connections
  while (!g_quit) {
    // set fd_read_set before select call
    FD_ZERO(&fd_read_set);
    FD_SET(server_socket, &fd_read_set);
    for (int i = 1; i <= max_connections; i++) {
      if (connections[i] != INVALID_SOCKET) {
        FD_SET(connections[i], &fd_read_set);
      }
    }

    // wait until select return
    ret_val = select(0, &fd_read_set, NULL, NULL, NULL);

    if (g_quit) {
      break;
    }

    if (ret_val > 0) {
      // check if socket with event is server_socket
      if (FD_ISSET(server_socket, &fd_read_set)) {
        // accept new client connection
        SOCKET client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == INVALID_SOCKET) {
          error_set_last_with_code(2, WSAGetLastError());
        }
        else {
          // add new client socket to the first free connections slot
          for (int i = 1; i <= max_connections; i++) {
            if (connections[i] == INVALID_SOCKET) {
              connections[i] = client_socket;
              if (g_sock_acc_cb)
                g_sock_acc_cb(connections[i]);
              break;
            }
          }
        }

        if (--ret_val == 0)
          continue;
      }

      // iterate connections in search of sockets with event
      for (int i = 1; i <= max_connections; i++) {
        if ((connections[i] != INVALID_SOCKET) && (FD_ISSET(connections[i], &fd_read_set))) {

          // run user defined callback function
          CB_RESULT res = CB_CLOSE_SOCKET;
          if (g_io_cb)
            res = g_io_cb(connections[i]);

          // react to callback result
          switch (res) {
          case CB_CONTINUE:
            break;
          case CB_CLOSE_SOCKET:
            connections[i] = INVALID_SOCKET; // invalidate connection
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
        error_set_last_with_code(3, WSAGetLastError());
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

    if (g_serv_close_cb)
      g_serv_close_cb();

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
    error_set_last(4, "init_winsock");
    return 0;
  }

  return 1;
}

void on_IO_request(IO_CALLBACK callback)
{
  g_io_cb = callback;
}

void on_socket_accept(SOCK_ACCEPT_CALLBACK callback)
{
  g_sock_acc_cb = callback;
}

void on_server_open(SERVER_OPEN_CALLBACK callback)
{
  g_serv_open_cb = callback;
}

void on_server_close(SERVER_CLOSE_CALLBACK callback)
{
  g_serv_close_cb = callback;
}