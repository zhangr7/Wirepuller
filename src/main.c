#include "wirepuller.h"

int main() {
    daemonize();
    int server_fd = create_server_socket();
    if (server_fd < 0) return 1;
    event_loop(server_fd);
    return 0;
}