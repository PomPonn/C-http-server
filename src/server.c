#include "server.h"

#include <ws2tcpip.h> // address resolving
#include <stdio.h> // console output

// inform compiler to use winsock library
#pragma comment(lib, "Ws2_32.lib")

// GLOBAL
int* connections;
CRITICAL_SECTION connections_mutex;
int conns_tracker = 0;

int handle_connections(SOCKET server_socket, int max_connections, client_callback callback) {
  // verify arguments
  if (max_connections < 1 || server_socket == INVALID_SOCKET || callback == NULL)
    return -1;

  // allocate memory for client sockets (+ server socket)
  connections = MemAlloc(0, sizeof(SOCKET) * (max_connections + 1));

  fd_set fd_read_set;
  SOCKET client_socket;
  int ret_val = 0;

  // init connections and let the first one be server socket
  connections[0] = server_socket;
  for (int i = 1; i < max_connections; i++) {
    connections[i] = -1;
  }

  SYSTEM_INFO system_info;
  TP_CALLBACK_ENVIRON callback_env;
  TP_POOL* tpool;

  // get number of CPU cores
  GetSystemInfo(&system_info);

  if (!(tpool = CreateThreadpool(NULL))) {
    printf("Failed to create thread pool: %d\n", (int)GetLastError());
    WSACleanup();
    MemFree(connections);
    return -1;
  }

  // set number of threads in the tpool
  SetThreadpoolThreadMaximum(tpool, system_info.dwNumberOfProcessors);
  if (!SetThreadpoolThreadMinimum(tpool, system_info.dwNumberOfProcessors)) {
    printf("Failed to set threads count in the thread pool: %d\n", (int)GetLastError());
    CloseThreadpool(tpool);
    WSACleanup();
    MemFree(connections);
    return -1;
  }

  // init thread pool callback environment
  InitializeThreadpoolEnvironment(&callback_env);
  SetThreadpoolCallbackPool(&callback_env, tpool);

  // init critical section
  InitializeCriticalSectionAndSpinCount(&connections_mutex, 6000);

  // start handling clients
  while (TRUE) {

    FD_ZERO(&fd_read_set);

    EnterCriticalSection(&connections_mutex);
    // fill fd_set before select call
    for (int i = 0; i < max_connections; i++) {
      if (connections[i] >= 0)
        FD_SET(connections[i], &fd_read_set);
    }
    LeaveCriticalSection(&connections_mutex);

    // wait until select return
    ret_val = select(FD_SETSIZE, &fd_read_set, NULL, NULL, NULL);

    if (ret_val > 0) {
      // check if socket with event is server_socket
      if (FD_ISSET(server_socket, &fd_read_set)) {
        // accept new connection
        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == INVALID_SOCKET) {
          printf("Accept failed\n");
        }
        else {
          // add the new client socket to the first free connections array slot
          for (int i = 1; i < max_connections; i++) {
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
      for (int i = 1; i < max_connections; i++) {
        if (connections[i] > 0 && FD_ISSET(connections[i], &fd_read_set)) {
          // init callback args
          VOID* callback_args =
            MemAlloc(HEAP_ZERO_MEMORY,
              sizeof(callback) + sizeof(int)/* conn index */);

          _CB_CALLBACK(callback_args) = callback;
          _CB_PARAM(callback_args, int, 0) = i;

          TP_WORK* work = CreateThreadpoolWork(_callback_wrapper, callback_args, &callback_env);
          SubmitThreadpoolWork(work);
          conns_tracker++;
          connections[i] = -1; // temporarly invalidate connection while thread is processing it
          // close work async when it wont be used anymore
          CloseThreadpoolWork(work);
        }
        // end loop if all connections have been found
        if (--ret_val == 0)
          break;
      }
    }
    else {
      printf("Accept failed: %d\n", WSAGetLastError());
    }
  } // while (TRUE)

  // cleanup (if there will be a way to get there)
  // wait for threads to finish first
  CloseThreadpool(tpool);
  DeleteCriticalSection(&connections_mutex);
  WSACleanup();

  return 0;
}

VOID CALLBACK _callback_wrapper(PTP_CALLBACK_INSTANCE instance, PVOID param, PTP_WORK work) {
  // extract data from param and free it
  client_callback callback = _CB_CALLBACK(param);
  int client_socket_index = _CB_PARAM(param, int, 0);

  MemFree(param);

  EnterCriticalSection(&connections_mutex);

  SOCKET client_socket = connections[client_socket_index];

  LeaveCriticalSection(&connections_mutex);

  // run user defined callback
  CB_RESULT res = callback(client_socket);

  // react to callback result
  switch (res) {
  case CB_CONTINUE:
    connections[client_socket_index] = client_socket; // validate connection back
    break;
  case CB_CLOSE_SOCKET:
    EnterCriticalSection(&connections_mutex);
    LeaveCriticalSection(&connections_mutex);

    closesocket(client_socket);
    break;
  }
}

SOCKET create_tcp_server_socket(char* port) {
  WSADATA wsa_data;

  // initialize winsock dll
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    printf("[server_socket ERROR]\tWSAStartup failed\n");
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
    printf("getaddrinfo failed with error code %d\n", err_code);
    WSACleanup();
    return INVALID_SOCKET;
  }

  // create socket
  SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (listen_socket == INVALID_SOCKET) {
    printf("Error at socket %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return INVALID_SOCKET;
  }

  // bind socket
  err_code = bind(listen_socket, result->ai_addr, result->ai_addrlen);
  if (err_code) {
    printf("Error at socket binding: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(listen_socket);
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