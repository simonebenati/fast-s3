#include <asm-generic/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <err.h>

void serve (uint16_t http_port, char *listen_addr) 
{
        struct sockaddr_in serv_addr = {0};
        struct sockaddr_in client_addr = {0};
        socklen_t client_len = sizeof(client_addr);
        int enable = 1;

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) err(EXIT_FAILURE, "socket");

        if (
                setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, client_len) == -1
        ) err(errno, "setsockopt");
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(http_port);
        if (inet_pton(AF_INET, listen_addr, &serv_addr.sin_addr.s_addr) != 1) err(EXIT_FAILURE, "inet_pton");

        if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) err(EXIT_FAILURE, "bind");

        printf("Now listening on port %d from only ip: %x\n", http_port, serv_addr.sin_addr.s_addr);

        if (listen(sock, 50) == -1) err(EXIT_FAILURE, "listen");

        int ret = 0;
        while(1) {
                ret = accept(sock, (struct sockaddr *) &client_addr, &client_len);
                if (ret == -1) exit(1);
                printf("Accepted client..\n");
        }
        close(sock);
        return;
}


int main() {
        serve(60443, "127.0.0.1");
        return 0;
}