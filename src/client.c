#include "wirepuller.h"

int main(int argc, char *argv[]) {
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }
    else {
        printf("Client connected.\n");
    }
    
    write(client_fd, argv[1], strlen(argv[1]));
    close(client_fd);
    return 0;
}