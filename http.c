#include <asm-generic/socket.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>

static void handle_sigchld(int sig_num)
{
        (void)sig_num;

        int saved_errno = errno;
        pid_t child_pid;
        while((child_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
                // Later do something with child pids.
        }
        return;
}

static void install_handle_sigchld(void)
{
        struct sigaction action = {0};

        action.sa_handler = handle_sigchld;
        action.sa_flags = SA_RESTART | SA_NOCLDSTOP;

        sigemptyset(&action.sa_mask);

        if (sigaction(SIGCHLD, &action, NULL) == -1) {
                perror("sigaction");
                exit(EXIT_FAILURE);
        }

        return;
}

void handle_client (int accept_fd, int listen_fd, struct sockaddr_in *client_fd) {
        pid_t pid = fork();
        switch (pid) {
        case -1:
                perror("handle_client-> fork");
                exit(EXIT_FAILURE);
                break;
        case 0:
                close(listen_fd);
                puts("Do something from child proc");
                sleep(100);
                close(accept_fd);
                _exit(EXIT_SUCCESS);
        default:
                close(accept_fd);
                puts("Do something...");
                break;
        }

        return;
}

void serve (uint16_t http_port, char *listen_addr) 
{
        struct sockaddr_in serv_addr = {0};
        struct sockaddr_in client_addr = {0};
        socklen_t client_len = 0;
        int enable = 1;
        int ret = 0;

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) err(EXIT_FAILURE, "socket");

        if (
                setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1
        ) err(EXIT_FAILURE, "setsockopt");
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(http_port);
        if (inet_pton(AF_INET, listen_addr, &serv_addr.sin_addr.s_addr) != 1) err(EXIT_FAILURE, "inet_pton");

        if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) err(EXIT_FAILURE, "bind");

        printf("Now listening on port %d from only ip: %x\n", http_port, serv_addr.sin_addr.s_addr);

        if (listen(sock, 50) == -1) err(EXIT_FAILURE, "listen");

        install_handle_sigchld();

        while(1) {
                client_len = sizeof(client_addr);
                ret = accept(sock, (struct sockaddr *) &client_addr, &client_len);

                if (ret == -1) exit(1);
                handle_client(ret, sock, &client_addr);
        }
        close(sock);
        return;
}


int main() {
        serve(60443, "127.0.0.1");
        return 0;
}