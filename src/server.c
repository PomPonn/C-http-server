#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdio.h>

// inform compiler to use winsock library
#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 1024

// use to set or get the callback parameter passed to callback wrapper through PVOID param
#define _CB_PARAM(callback_args, param_type) (*(param_type*)((BYTE*)callback_args + sizeof(handle_client)))
// use to set or get the callback pointer passed to callback wrapper through PVOID param
#define _CB_CALLBACK(callback_args) (*(void (**)(SOCKET))callback_args)

char* _get_buffer_line(char* buffer, char** last_char) {
  char* line, * ptr = buffer;
  int line_size = 0;

  // find size line length ( check for \r\n string [CRLF])
  while (*ptr != '\r' || *(ptr + 1) != '\n') {
    ptr++;
    line_size++;
  }

  line = HeapAlloc(GetProcessHeap(), 0, line_size);
  ptr = buffer;

  for (int i = 0; i < line_size; i++) {
    line[i] = *ptr;
    ptr++;
  }
  line[line_size] = '\0';

  *last_char = ptr + 2;

  return line;
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
    printf("[server_socket ERROR]\tgetaddrinfo failed with error code %d\n", err_code);
    WSACleanup();
    return INVALID_SOCKET;
  }

  // create socket
  SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (listen_socket == INVALID_SOCKET) {
    printf("[server_socket ERROR]\tError at socket %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return INVALID_SOCKET;
  }

  // bind socket
  err_code = bind(listen_socket, result->ai_addr, result->ai_addrlen);
  if (err_code) {
    printf("[server_socket ERROR]\tError at socket binding: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(listen_socket);
    WSACleanup();
    return INVALID_SOCKET;
  }

  freeaddrinfo(result);

  // listen for connections
  if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
    printf("[server_socket ERROR]\tListen failed with error: %d\n", WSAGetLastError());
    closesocket(listen_socket);
    WSACleanup();
    return INVALID_SOCKET;
  }

  return listen_socket;
}

VOID CALLBACK _handle_client_wrapper(PTP_CALLBACK_INSTANCE instance, PVOID param, PTP_WORK work) {
  // extract data from param and free it
  void (*handle_client)(SOCKET client_socket) = _CB_CALLBACK(param);
  SOCKET client_socket = _CB_PARAM(param, SOCKET);

  HeapFree(GetProcessHeap(), 0, param);

  // run user defined callback
  handle_client(client_socket);
}

int handle_connections(SOCKET server_socket, int max_connections, void (*handle_client)(SOCKET client_socket)) {
  if (max_connections < 1 || server_socket == INVALID_SOCKET || handle_client == NULL)
    return -1;

  HANDLE proc_heap = GetProcessHeap();
  // allocate memory for client sockets (+ server socket)
  int* connections = HeapAlloc(proc_heap, 0, sizeof(SOCKET) * (max_connections + 1));

  // declare file descriptors set
  fd_set fd_read_set;
  SOCKET client_socket;
  int ret_val = 0;

  // init connections and let the first one be server socket
  for (int i = 0; i < max_connections; i++) {
    connections[i] = -1;
  }
  connections[0] = server_socket;

  // INIT THREAD POOL
  SYSTEM_INFO system_info;
  TP_CALLBACK_ENVIRON callback_env;
  TP_POOL* tpool;

  // get number of CPU cores
  GetSystemInfo(&system_info);

  if (!(tpool = CreateThreadpool(NULL))) {
    printf("Failed to create thread pool: %d\n", GetLastError());
    WSACleanup();
    HeapFree(proc_heap, 0, connections);
    return -1;
  }

  // set number of threads in the pool
  SetThreadpoolThreadMaximum(tpool, system_info.dwNumberOfProcessors);
  if (!SetThreadpoolThreadMinimum(tpool, system_info.dwNumberOfProcessors)) {
    printf("Failed to set threads count in the thread pool: %d\n", GetLastError());
    CloseThreadpool(tpool);
    WSACleanup();
    HeapFree(proc_heap, 0, connections);
    return -1;
  }

  // init thread pool callback environment
  InitializeThreadpoolEnvironment(&callback_env);

  SetThreadpoolCallbackPool(&callback_env, tpool);

  // START HANDLING CLIENTS
  while (TRUE) {
    FD_ZERO(&fd_read_set);
    // fill fd_set before select call
    for (int i = 0; i < max_connections; i++) {
      if (connections[i] >= 0)
        FD_SET(connections[i], &fd_read_set);
    }

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
          VOID* callback_args = HeapAlloc(proc_heap, HEAP_ZERO_MEMORY,
            sizeof(handle_client) + sizeof(SOCKET) /*param*/);

          _CB_CALLBACK(callback_args) = handle_client;
          _CB_PARAM(callback_args, SOCKET) = connections[i];

          TP_WORK* work = CreateThreadpoolWork(_handle_client_wrapper, callback_args, &callback_env);
          SubmitThreadpoolWork(work);
          connections[i] = -1; // TESTING : DONt WORK
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
  CloseThreadpool(tpool);
  WSACleanup();

  return 0;
}