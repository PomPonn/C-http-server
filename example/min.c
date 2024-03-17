#include "server/http_server.h"

void on_request(http_request* req, http_response* res) {
  if (str_is_equal(req->url_path, "/")) {

    *res = http_build_response(
      req->variant,
      req->version,
      "200 OK",
      (http_header[]) {
        { "Content-Type", "text/plain" },
        { "Content-Length", "13" }
    },
      2,
      "Hello, world!",
      (void*)0
    );
  }
}

int main() {
  SOCKET server = http_create_server("localhost", "3000");

  http_server_listen(server, 100, on_request);

  return 0;
}