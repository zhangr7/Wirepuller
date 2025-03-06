#ifndef WIREPULLER_H
#define WIREPULLER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/types.h>

#define SOCKET_PATH "/tmp/wirepuller.sock"
#define MAX_EVENTS 10
#define MAX_SERVICES 100

// Structure for storing registered services
typedef struct {
    char name[256];
    int client_fd;
} Service;

struct ucred {
    pid_t pid;
    uid_t uid;
    gid_t gid;
};

extern Service service_list[MAX_SERVICES];
extern int service_count;

// Function declarations
void daemonize();
int create_server_socket();
void event_loop(int server_fd);
void handle_client(int client_fd);
void send_fd(int socket, int fd_to_send);
bool register_service(const char *service_name, int client_fd);
void list_services(int client_fd);
int find_service(const char *service_name, int *client_fd);

#endif