#include "server/base_server.h"

#include "misc/error.h"
#include "misc/utils.h"
#include "cp_defs/srv.h"

// change if needed
#define FD_SETSIZE 1024

#ifdef _WIN32

#define WINDOWS_LEAN_AND_MEAN
#include <WinSock2.h> // socket library
#include <ws2tcpip.h> // address resolve

// inform compiler to use winsock library
#pragma comment(lib, "Ws2_32.lib")

#elif __linux__

#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#endif

// global variables for use in control handler
SOCKET* g_connections = NULL;
int g_max_conns = 1;
int g_quit = FALSE, g_processing = FALSE;

// callbacks
IO_CALLBACK g_io_cb = NULL;
SOCK_ACCEPT_CALLBACK g_sock_acc_cb = NULL;
SERVER_CLOSE_CALLBACK g_serv_close_cb = NULL;
SERVER_OPEN_CALLBACK g_serv_open_cb = NULL;

#ifdef _WIN32
BOOL WINAPI _control_handler(DWORD ctrl_type);
#else
void _at_exit();
#endif

int start_listening(SOCKET server_socket, int max_connections) {
  // verify arguments
  if (server_socket == INVALID_SOCKET || max_connections < 1) {
    error_set_last(1, "create_server");
    goto __err_cleanup;
  }

  // for error handling
  char temp[TEMP_SIZE];

  // listen for connections
  if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
    error_set_last_with_code(7, default_last_err);
    goto __err_cleanup;
  }

  g_max_conns = max_connections;
  fd_set fd_read_set;
  int ret_val = 0;

  // set control handler to make cleanup after server shutdown
  #ifdef _WIN32
  if (!SetConsoleCtrlHandler(_control_handler, TRUE)) {
    error_set_last_with_code(14, GetLastError());
    goto __err_cleanup;
  }
  #else
  if (atexit(_at_exit)) {
    error_set_last_with_code(14, errno);
    goto __err_cleanup;
  }
  #endif

  // allocate memory for client sockets (+1 for server socket)
  g_connections = malloc(sizeof(SOCKET) * (max_connections + 1));

  // init connections and let the first one be the server socket
  g_connections[0] = server_socket;
  for (int i = 1; i <= max_connections; i++) {
    g_connections[i] = INVALID_SOCKET;
  }

  // call on server startup callback
  if (g_serv_open_cb)
    g_serv_open_cb();

  //
  // Start connections loop
  //
  while (!g_quit) {
    g_processing = FALSE;

    // set fd_read_set before every select call
    FD_ZERO(&fd_read_set);
    FD_SET(server_socket, &fd_read_set);
    for (int i = 1; i <= max_connections; i++) {
      if (g_connections[i] != INVALID_SOCKET) {
        FD_SET(g_connections[i], &fd_read_set);
      }
    }

    // wait until select return
    ret_val = select(0, &fd_read_set, NULL, NULL, NULL);

    if (g_quit)
      break;

    if (ret_val < 0) {
      error_set_last_with_code(3, default_last_err);
    }
    if (ret_val == 0) {
      error_set_last(13, "select");
    }
    else {
      g_processing = TRUE;

      // check if socket with event is server_socket
      if (FD_ISSET(server_socket, &fd_read_set)) {
        // accept new client connection
        SOCKET client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == INVALID_SOCKET) {
          error_set_last_with_code(2, default_last_err);
        }
        else {
          // add new client socket to the first free connections slot
          for (int i = 1; i <= max_connections; i++) {
            if (g_connections[i] == INVALID_SOCKET) {
              g_connections[i] = client_socket;
              // run on new connection callback
              if (g_sock_acc_cb)
                g_sock_acc_cb(g_connections[i]);
              break;
            }
          }
        }

        // continue if there are no more connections
        if (--ret_val == 0)
          continue;
      }

      // iterate connections in search of sockets with event
      for (int i = 1; i <= max_connections; i++) {
        if ((g_connections[i] != INVALID_SOCKET) && (FD_ISSET(g_connections[i], &fd_read_set))) {

          CB_RESULT res = CB_CLOSE_SOCKET;
          // run on request callback
          if (g_io_cb)
            res = g_io_cb(g_connections[i]);

          // react to callback result
          switch (res) {
          case CB_CONTINUE:
            break;
          case CB_CLOSE_SOCKET:
            g_connections[i] = INVALID_SOCKET; // invalidate connection
            break;
          }

          // end loop if all connections have been found
          if (--ret_val == 0)
            break;
        }
      }
    }
  } // while (TRUE)

  return 0;

__err_cleanup:
  socket_close(server_socket);
  cleanup();
  return -1;
}

void on_IO_request(IO_CALLBACK callback) {
  g_io_cb = callback;
}

void on_socket_accept(SOCK_ACCEPT_CALLBACK callback) {
  g_sock_acc_cb = callback;
}

void on_server_open(SERVER_OPEN_CALLBACK callback) {
  g_serv_open_cb = callback;
}

void on_server_close(SERVER_CLOSE_CALLBACK callback) {
  g_serv_close_cb = callback;
}


SOCKET create_server_socket
(const char* host, const char* port, int server_socket_type, int protocol) {

  SOCKET server_socket = INVALID_SOCKET;
  struct addrinfo hints, * result = NULL;
  // for error handling
  char temp[TEMP_SIZE];

  // set hints struct
  #ifdef _WIN32
  ZeroMemory(&hints, sizeof(hints));
  #elif __linux__
  bzero(&hints, sizeof(hints));
  #endif
  hints.ai_family = AF_UNSPEC;
  hints.ai_protocol = protocol;
  hints.ai_socktype = server_socket_type;
  hints.ai_flags = AI_PASSIVE;

  int retval = getaddrinfo(host, port, &hints, &result);

  // verify whether getaddrinfo failed
  if (retval || !result) {
    error_set_last_with_code(5, default_last_err);
    goto __err_cleanup;
  }

  // Create server socket
  server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (server_socket == INVALID_SOCKET)
  {
    error_set_last_with_code(6, default_last_err);
    goto __err_cleanup;
  }

  // Bind the socket
  retval = bind(server_socket, result->ai_addr, result->ai_addrlen);
  if (retval == SOCKET_ERROR)
  {
    error_set_last_with_code(11, default_last_err);
    goto __err_cleanup;
  }


  // Make the socket non-blocking
  #ifdef _WIN32
  u_long io_mode = 1;
  retval = ioctlsocket(server_socket, FIONBIO, &io_mode);
  #elif __linux__
  retval = fcntl(server_socket, F_SETFL, O_NONBLOCK);
  #endif
  if (retval == SOCKET_ERROR)
  {
    error_set_last_with_code(12, default_last_err);
    goto __err_cleanup;
  }

  freeaddrinfo(result);

  return server_socket;

__err_cleanup:
  if (result)
    freeaddrinfo(result);
  if (server_socket != INVALID_SOCKET)
    socket_close(server_socket);
  cleanup();
  return INVALID_SOCKET;
}

#ifdef _WIN32

BOOL init_winsock() {
  WSADATA wsa_data;

  // initialize winsock dll
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    error_set_last(4, "init_winsock");
    return 0;
  }

  return 1;
}

BOOL WINAPI _control_handler(DWORD ctrl_type) {
  switch (ctrl_type)
  {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_SHUTDOWN_EVENT:
  case CTRL_CLOSE_EVENT:
  {
    g_quit = TRUE;

    // wait until server idle
    while (g_processing) {}

    // free and close connections
    for (int i = 0; i <= g_max_conns; i++) {
      if (g_connections[i] != INVALID_SOCKET) {
        socket_close(g_connections[i]);
      }
    }
    free(g_connections);

    // run on server shutdown callback
    if (g_serv_close_cb)
      g_serv_close_cb();

    cleanup();

    ExitProcess(0);
    return TRUE;
  }
  default:
    return FALSE;
  }
}

#elif __linux__

#endif

#ifndef _WIN32

void _at_exit() {
  g_quit = TRUE;
  printf("at exit works\n"); // DELETE
  // wait until server idle
  while (g_processing) {}

  // free and close connections
  for (int i = 0; i <= g_max_conns; i++) {
    if (g_connections[i] != INVALID_SOCKET) {
      socket_close(g_connections[i]);
    }
  }
  free(g_connections);

  // run on server shutdown callback
  if (g_serv_close_cb)
    g_serv_close_cb();
}

#endif