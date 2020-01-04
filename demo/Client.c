#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IPADDR  "192.168.8.101"
#define SERVER_PORT    8086

int main() {

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd <= 0) {
		printf("socket failed\n");
		exit(-1);
	}

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IPADDR);

    int ret = connect(clientfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret < 0) {
        printf("connect fail\n");
        exit(-1);
    }

    char buffer[1024] = {0};
    ssize_t n;

    while(1) {
        n = read(STDIN_FILENO, buffer, sizeof(buffer));
        if(n > 0) {
            n = write(clientfd, buffer, n);
            if(n < 0) {
                printf("write error\n");
                break;
            }
            n = read(clientfd, buffer, sizeof(buffer));
            write(STDOUT_FILENO, buffer, n);

        } else if(n == 0) break;
    }

    return 0;
}
