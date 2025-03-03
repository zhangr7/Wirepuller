#include "wirepuller.h"

void handle_client(int client_fd) {
    char buffer[256];
    int bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Received request: %s\n", buffer);
    }
    close(client_fd);
}