#include "server.h"

#define DEFAULT_PORT "80"

int main() {
    server_t server = create_server(DEFAULT_PORT);

    listen_on(server, NULL);

    return 0;
}