# Wirepuller
Project by Robert Zhang

# Overview
**Wirepuller** is a simple UNIX daemon in C that functions as a nameserver, enabling clients to find each other based on what services they offer (identified with a string),
The name of the service is taken from the antiquated roles of telephone operators who were intermediaries that connected callers to each other through the use of a central switchboard.
Wirepulling was the physical installation of telephone wires to their right connections.

The daemon enables clients to register services with a string identifier and allows other clients to discover them.
Architecturally, the source code for this project serves the following purposes:
- wirepuller.h: Header file for shared declarations
- daemon.c: Daemonization logic
- server.c: Main server logic (socket setup, event loop)
- client.c: Command-line client implementation
- client_handler.c: Handles client requests
- fd_passing.c: File descriptor passing helper functions
- main.c: Entry point to start the daemon

This project implements the following 5 requirements in its design:
1. Use UNIX domain sockets for communication with a known path for the name of the socket.
2. Use accept to create a connection-per-client..
3. Use event notification (epoll, poll, select, etc...) to manage concurrency.
4. Use domain socket facilities to get a trustworthy identity of the client (i.e. user id).
5. Pass file descriptors between the service and the client.

## UNIX Domain Sockets
Domain sockets are used for local interprocess communication. In wirepuller.h we define the known socket path:
```
#define SOCKET_PATH "/tmp/wirepuller.sock"
```
Both server.c and client.c utilize this path when creating socket connections.

**server.c**
We use socket() to return a file descriptor for the server socket.
```
int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
```

Then we store the socket path into sockaddr_un, a struct used to define addresses for domain sockets.
```
struct sockaddr_un addr = {0};
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
```

Finally, we bind the domain socket to the path address and use the listen system call to mark the socket as passive in order to receive incoming connections.
```
if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind failed");
    close(server_fd);
    return -1;
}
```
**client.c**
We use socket() to return a file descriptor for the client socket.
`int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);`

Then we store the socket path into sockaddr_un, a struct used to define addresses for domain sockets.
```
struct sockaddr_un addr = {0};
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
```

Finally we connect the client to the domain socket address.
```
if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("connect failed");
    close(client_fd);
    return 1;
}
```

## Accept Usage
In the event loop of server.c, we call the accept() system call to accept an incoming connection and return a new socket connection between the server and the client.
```
int client_fd = accept(server_fd, NULL, NULL);
```

## Event Notification 
The event loop in server.c uses epoll to monitor for client connections.

We first create an epoll instance.
```
int epoll_fd = epoll_create1(0);
```

Then we add the server file descriptor to epoll so we can detect new client connections.
```
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
```

We use epoll_wait() in the infinite loop to block until a file descriptor is ready.
```
while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
```

Using accept as described above, when a client is connected we add to epoll to monitor messages from this client.
```
if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
    perror("epoll_ctl add client failed");
    close(client_fd);
    continue;
}
```

When client requests come in we handle those requests with handle_client() which will be elaborated in the sections below.
```
handle_client(events[i].data.fd);
```

## Identifying Clients 
In handle_client() in client_handler.c we use getsockopt() from SO_PEERCRED to ensure local user identity by returning various ID's.

```
struct ucred client_cred;
socklen_t cred_length = sizeof(client_cred);

// Retrieve client credentials using SO_PEERCRED
if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &client_cred, &cred_length) == 0) {
    printf("Client connected - PID: %d, UID: %d, GID: %d\n", client_cred.pid, client_cred.uid, client_cred.gid);
} else {
    perror("Failed to get client credentials");
    return;
}
```

## Passing File Descriptors 
We pass a file descriptor from server to client in fd_passer.c with sendmsg() to pass the fd over the UNIX domain socket.
```
void send_fd(int socket, int fd_to_send) {
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char buf[CMSG_SPACE(sizeof(int))];
    
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    
    memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(int));
    sendmsg(socket, &msg, 0);
}
```

This is implemented in client_handler.c when we want to find a service. The daemon retrieves the FD of the client offereing the service.
```
 if (find_service(buffer + 5, &found_fd) == 0) {
    char response[512];
    snprintf(response, sizeof(response), "Service found under client FD: %d\n", found_fd);
    write(client_fd, response, strlen(response));
    send_fd(client_fd, found_fd);
```

# Testing Procedure
Compile the server and client with the following commands in the terminal:
> gcc src/main.c src/daemon.c src/client_handler.c src/fd_passer.c src/server.c -Wall -o wirepuller
> gcc src/client.c -Wall -o client

Start the server.
> ./wirepuller

Start the client.
> ./client

Simulate multiple clients by opening a new terminal and starting the client as above.

After successfully starting the client you will be prompted to enter commands. Note the total number of events (commands) is capped at 10. If you would like to continue testing past that you need to restart the testing procedure.

Register a service using this command:
> register your_service_name

List all the clients and their services using this command:
> list

You can look for a specific service using this command:
> find your_service_name

You can close the client connection by typing:
`exit`