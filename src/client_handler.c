#include "wirepuller.h"

Service service_list[MAX_SERVICES];
int service_count = 0;

bool register_service(const char *service_name, int client_fd) {
    if (service_count >= MAX_SERVICES) {
        return false;
    }
    strncpy(service_list[service_count].name, service_name, sizeof(service_list[service_count].name) - 1);
    service_list[service_count].client_fd = client_fd;
    service_count++;
    return true;
}

void list_services(int client_fd) {
    char buffer[1024] = "Registered Services:\n";
    for (int i = 0; i < service_count; i++) {
        char entry[512];
        snprintf(entry, sizeof(entry), "client FD: %d, Service: %s\n", service_list[i].client_fd, service_list[i].name);
        strcat(buffer, entry);
    }
    write(client_fd, buffer, strlen(buffer));
}

int find_service(const char *service_name, int *client_fd) {
    for (int i = 0; i < service_count; i++) {
        if (strcmp(service_list[i].name, service_name) == 0) {
            *client_fd = service_list[i].client_fd;
            return 0;
        }
    }
    return -1;
}

void handle_client(int client_fd) {
    struct ucred client_cred;
    socklen_t cred_length = sizeof(client_cred);

    // Retrieve client credentials using SO_PEERCRED
    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &client_cred, &cred_length) == 0) {
        printf("Client connected - PID: %d, UID: %d, GID: %d\n", client_cred.pid, client_cred.uid, client_cred.gid);
    } else {
        perror("Failed to get client credentials");
        return;
    }
    
    char buffer[256];
    int bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Received request: %s\n", buffer);
        
        if (strncmp(buffer, "register ", 9) == 0) {
            if (register_service(buffer + 9, client_fd)) {
                write(client_fd, "Service registered successfully\n", 32);
            } else {
                write(client_fd, "Service registration failed\n", 29);
            }
        } else if (strncmp(buffer, "list", 4) == 0) {
            list_services(client_fd);
        } else if (strncmp(buffer, "find ", 5) == 0) {
            int found_fd;
            if (find_service(buffer + 5, &found_fd) == 0) {
                char response[512];
                snprintf(response, sizeof(response), "Service found under client FD: %d\n", found_fd);
                write(client_fd, response, strlen(response));
                send_fd(client_fd, found_fd);
            } else {
                write(client_fd, "Service not found\n", 19);
            }
        } else {
            write(client_fd, "Unknown command\n", 16);
        }
    }
}