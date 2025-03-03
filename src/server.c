#include "wirepuller.h"

int create_server_socket() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) return -1;
    
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    unlink(SOCKET_PATH);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;
    if (listen(server_fd, 10) < 0) return -1;

    printf("Server is listening.\n");
    return server_fd;
}

void event_loop(int server_fd) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) return;
    
    struct epoll_event event = { .events = EPOLLIN, .data.fd = server_fd };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
    
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd >= 0) {
                    printf("Accepted new client connection: %d\n", client_fd);
                }
                else {
                    perror("Accept failed.\n");
                }
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
            } else {
                printf("Handling client with FD: %d\n", events[i].data.fd);
                handle_client(events[i].data.fd);
            }
        }
    }
}