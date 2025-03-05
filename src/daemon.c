#include "wirepuller.h"

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    if (setsid() < 0) exit(EXIT_FAILURE);
    
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    printf("Daemon is running.\n");

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    umask(0);
    chdir("/");
    
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) close(x);
}