#include <arpa/inet.h>

#include "coroutine.h"

void handler(void *arg) {
    int fd = *(int *)arg;
    ssize_t n;
    char buffer[1024] = {0};

    while(1) {

        n = coroutine_read(fd, buffer, 1024);
        if(n > 0) {
            n = coroutine_write(fd, buffer, n);
            if(n < 0) {
                close(fd);
                break;
            }

        } else if (n == 0) {
            close(fd);
            break;
        }
    }
}

void server(void *arg) {
    unsigned int port = *(unsigned short *)arg;
    socklen_t len;
    int cli_fd;
    
    int sockfd = coroutine_socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) return;

    struct sockaddr_in servaddr, cliaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    listen(sockfd, 128);
    
    while(1)
    {
        cli_fd = coroutine_accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        printf("%s connect server\n", inet_ntoa(cliaddr.sin_addr));
        coroutine_t *co;
        coroutine_create(&co, handler, (void *)&cli_fd);
    }

}

int main() {
    coroutine_t *co = NULL;
    unsigned int port = 8086;

    coroutine_create(&co, server, (void *)&port);

    // start running
    schedule_running();

    return 0;
}