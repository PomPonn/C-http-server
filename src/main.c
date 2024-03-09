#include "server.h"
#include "stdio.h"

CB_RESULT callback(SOCKET client_socket) {
  char buffer[1024];

  int ret_val = recv(client_socket, (char*)buffer, sizeof(buffer), 0);

  if (ret_val < 0) {
    printf("recv error: %d\n", ret_val);
  }
  else if (ret_val == 0) {
    printf("socket: %d: connection closed\n", (int)client_socket);
    return CB_CLOSE_SOCKET;
  }
  else {
    printf("socket: %d: connection established\n", (int)client_socket);
    printf("socket: %d: bytes received: %d\n", (int)client_socket, ret_val);
    send(client_socket, buffer, sizeof(buffer), 0);
  }

  return CB_CONTINUE;
}

int main() {
  init_winsock();

  SOCKET listener = create_listen_socket("localhost", "80", SOCK_STREAM, AF_INET, IPPROTO_TCP);

  handle_connections(listener, 100, callback);

  return 0;
}