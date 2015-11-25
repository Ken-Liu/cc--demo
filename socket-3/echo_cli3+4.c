#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define ERR_EXIT(m) \
        do{         \
            perror(m);\
	    exit(EXIT_FAILURE);\
        }while(0) 
int main(void)
{
    int sock_fd;
    if( (sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_port = htons(5566);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if( connect(sock_fd, (struct sockaddr *)&srv_addr,sizeof(srv_addr)) < 0)
        ERR_EXIT("connect");
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};
    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        int ret = write(sock_fd, sendbuf, strlen(sendbuf));
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            close(sock_fd);
            ERR_EXIT("write");
        }else if(ret == 0)
            continue;
        memset(recvbuf, 0, sizeof(recvbuf));
        ret = read(sock_fd, recvbuf, sizeof(recvbuf));
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            close(sock_fd);
            ERR_EXIT("read");
        }else if(ret == 0)
        {
            printf("server is close");
            break;
        }
        fputs(recvbuf, stdout);
    }
    close(sock_fd);
}
