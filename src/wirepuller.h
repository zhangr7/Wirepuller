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

#define SOCKET_PATH "/tmp/wirepuller.sock"
#define MAX_EVENTS 10

void daemonize();
int create_server_socket();
void event_loop(int server_fd);
void handle_client(int client_fd);
void send_fd(int socket, int fd_to_send);

#endif