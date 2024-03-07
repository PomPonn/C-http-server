#include "server.h"
#include "stdio.h"

#define DEFAULT_PORT "80"

#define BUFLEN 500
/*
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
        printf("[server_socket ERROR]\tsend failed: %d\n", WSAGetLastError());
        closesocket(send_result);
        WSACleanup();
        return 1;
      }
    }
    else if (recv_result == 0) {
      printf("[server_socket LOG]\tConnection closing...\n");
    }
    else {
      printf("[server_socket ERROR]\trecv failed: %d\n", WSAGetLastError());
      closesocket(client_socket);
      WSACleanup();
      return 1;
    }
  } while (recv_result > 0);

  // shutdown the connection
  int err_code = shutdown(client_socket, SD_BOTH);
  if (err_code == SOCKET_ERROR) {
    printf("[server_socket ERROR]\tShutdown failed: %d\n", WSAGetLastError());
    closesocket(client_socket);
    WSACleanup();
    return 1;
  }

  // cleanup
  closesocket(client_socket);

  return 0;
}
*/
void callback(SOCKET client_socket) {
  char buffer[1024];

  int ret_val = recv(client_socket, (char*)buffer, sizeof(buffer), 0);


  if (ret_val < 0) {
    printf("recv error: %d\n", ret_val);
  }
  else if (ret_val == 0) {
    printf("socket: %d: connection closed\n", client_socket);
    closesocket(client_socket);
  }
  else {
    printf("socket: %d: connection established\n", client_socket);
    printf("socket: %d: bytes received: %d\n", client_socket, ret_val);
    send(client_socket, buffer, sizeof(buffer), 0);
  }
}

int main() {
  SOCKET server = create_tcp_server_socket(DEFAULT_PORT);

  handle_connections(server, 64, callback);


  return 0;
}