#include "wirepuller.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [register|find|list] <service_name>\n", argv[0]);
        return 1;
    }
    
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket failed");
        return 1;
    }
    
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        close(client_fd);
        return 1;
    }
    
    if (strcmp(argv[1], "register") == 0 && argc == 3) {
        dprintf(client_fd, "register %s", argv[2]);
    } else if (strcmp(argv[1], "find") == 0 && argc == 3) {
        dprintf(client_fd, "find %s", argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        dprintf(client_fd, "list");
    } else {
        printf("Invalid command. Use register, find, or list.\n");
    }
    
    char response[256];
    int bytes_read = read(client_fd, response, sizeof(response) - 1);
    if (bytes_read > 0) {
        response[bytes_read] = '\0';
        printf("%s\n", response);
    }
    
    close(client_fd);
    return 0;
}