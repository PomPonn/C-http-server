#include "server.h"

#include <ws2tcpip.h> // address resolve
#include <stdio.h> // console output

// inform compiler to use winsock library
#pragma comment(lib, "Ws2_32.lib")

// GLOBAL
int* connections;
CRITICAL_SECTION conns_mutex;

int handle_connections(SOCKET server_socket, int max_connections, client_callback callback) {
  // verify parameters
  if (max_connections < 1 || server_socket == INVALID_SOCKET || callback == NULL)
    return -1;

  fd_set fd_read_set;
  SOCKET client_socket;
  SYSTEM_INFO system_info;
  TP_CALLBACK_ENVIRON callback_env;
  TP_POOL* tpool;
  int ret_val = 0;

  // allocate memory for client sockets (+1 server socket)
  connections = MemAlloc(0, sizeof(SOCKET) * (max_connections + 1));

  // init connections and let the first one be the server socket
  // -1 value is reserved for sockets being unavailable to use, so fill connections with -2 instead
  connections[0] = server_socket;
  for (int i = 1; i <= max_connections; i++) {
    connections[i] = -2;
  }

  // init connections criical section
  InitializeCriticalSectionAndSpinCount(&conns_mutex, 6000);

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

  // start handling clients
  while (TRUE) {
    // reset fd_read_set before select call
    FD_ZERO(&fd_read_set);
    EnterCriticalSection(&conns_mutex);
    for (int i = 0; i <= max_connections; i++) {
      if (connections[i] >= 0)
        FD_SET(connections[i], &fd_read_set);
    }
    LeaveCriticalSection(&conns_mutex);

    // wait until select return
    ret_val = select(0, &fd_read_set, NULL, NULL, NULL);
    printf("\nre_val: %d", ret_val);

    if (ret_val > 0) {
      // check if socket with event is server_socket
      if (FD_ISSET(server_socket, &fd_read_set)) {
        printf(" server");
        // accept new client connection
        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == INVALID_SOCKET) {
          printf("Accept failed: %d\n", WSAGetLastError());
        }
        else {
          // add new client socket to the first free connections slot
          for (int i = 1; i < max_connections; i++) {
            if (connections[i] < -1) {
              connections[i] = client_socket;
              break;
            }
          }
        }

        if (--ret_val == 0)
          continue;
      }

      printf(" client\n");
      // iterate connections in search of sockets with event
      for (int i = 1; i <= max_connections; i++) {
        if (connections[i] > 0 && FD_ISSET(connections[i], &fd_read_set)) {
          // init callback args
          VOID* callback_args =
            MemAlloc(HEAP_ZERO_MEMORY,
              sizeof(callback) + sizeof(SOCKET)/* client_socket */ + sizeof(int)/* conn index */);

          _CB_CALLBACK(callback_args) = callback;
          _CB_PARAM(callback_args, SOCKET, 0) = connections[i];
          _CB_PARAM(callback_args, int, sizeof(SOCKET)) = i;

          // create and submit work
          TP_WORK* work = CreateThreadpoolWork(_callback_wrapper, callback_args, &callback_env);
          SubmitThreadpoolWork(work);

          EnterCriticalSection(&conns_mutex);
          connections[i] = -1; // temporarily invalidate connection while thread is processing it
          LeaveCriticalSection(&conns_mutex);

          // close work async when it wont be used anymore
          CloseThreadpoolWork(work);

          // end loop if all connections have been found
          if (--ret_val == 0)
            break;
        }
      }
    }
    else {
      printf("Select failed: %d\n", WSAGetLastError());
    }
  } // while (TRUE)

  // cleanup (if there will be a way to get there)
  // wait for threads to finish first

   /* Close all sockets */
  for (int i = 0; i < max_connections; i++) {
    if (connections[i] > 0) {
      closesocket(connections[i]);
    }
  }
  DestroyThreadpoolEnvironment(&callback_env);
  CloseThreadpool(tpool);
  WSACleanup();

  return 0;
}

VOID CALLBACK _callback_wrapper(PTP_CALLBACK_INSTANCE instance, PVOID param, PTP_WORK work) {
  // extract data from param and free it
  client_callback callback = _CB_CALLBACK(param);
  SOCKET client_socket = _CB_PARAM(param, SOCKET, 0);
  int client_socket_index = _CB_PARAM(param, int, sizeof(SOCKET));

  MemFree(param);

  // run user defined callback
  CB_RESULT res = callback(client_socket);

  // react to callback result
  EnterCriticalSection(&conns_mutex);
  switch (res) {
  case CB_CONTINUE:
    connections[client_socket_index] = client_socket; // open connection back
    break;
  case CB_CLOSE_SOCKET:
    connections[client_socket_index] = -2; // invalidate connection
    closesocket(client_socket);
    break;
  }
  LeaveCriticalSection(&conns_mutex);
}

// to remake

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