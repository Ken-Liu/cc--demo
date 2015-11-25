#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#define ERR_EXIT(m) \
         do{        \
             perror(m); \
             exit(EXIT_FAILURE);\
         }while(0)
int main(void)
{
    int sock_fd;
    if( (sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(5566);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_fd,(struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
	ERR_EXIT("bind");
    if(listen(sock_fd, SOMAXCONN) < 0)
	ERR_EXIT("listen");
    int conn;
    int len = sizeof(srv_addr);
    if( (conn = accept(sock_fd, (struct sockaddr *)&srv_addr, &len)) < 0)  
        ERR_EXIT("accept");
    int recvbuf[1024] = {0};
    int sendbuf[1024] = {0};
    while(1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(conn, recvbuf, sizeof(recvbuf));
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            close(sock_fd);
            close(conn);
            ERR_EXIT("read");
        }else if(ret == 0)
        {
            printf("Client is closed");
            break;
        }
        fputs(recvbuf, stdout);
        ret = write(conn, recvbuf, strlen(recvbuf));
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;//this have some trouble, it needs run from head of thewhile
            break;
        }else if(ret == 0)
                  continue;
    }
    close(sock_fd);
    close(conn);	        
}
