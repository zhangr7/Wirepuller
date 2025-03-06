#include "wirepuller.h"

void send_request(int client_fd, const char *command) {
    if (write(client_fd, command, strlen(command)) < 0) {
        perror("write failed");
        return;
    }

    char response[256];
    int bytes_read = read(client_fd, response, sizeof(response) - 1);
    if (bytes_read > 0) {
        response[bytes_read] = '\0';
        printf("%s\n", response);
    } else {
        perror("read failed");
    }
}

int main() {
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

    printf("Connected to the Wirepuller. Type 'exit' to quit.\n");

    // Interactive mode for continuous input
    char input[256], command[256];

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        // Remove newline character
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Parse user input into supported commands
        if (strncmp(input, "register ", 9) == 0) {
            snprintf(command, sizeof(command), "register %s", input + 9);
        } else if (strncmp(input, "find ", 5) == 0) {
            snprintf(command, sizeof(command), "find %s", input + 5);
        } else if (strcmp(input, "list") == 0) {
            snprintf(command, sizeof(command), "list");
        } else {
            printf("Invalid command. Use 'register <service>', 'find <service>', or 'list'.\n");
            continue;
        }

        send_request(client_fd, command);
    }

    close(client_fd);
    printf("Disconnected from the nameserver.\n");
    return 0;
}