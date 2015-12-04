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
size_t nread(int fd, char * buf, ssize_t len)
{
    memset(buf, 0, len);
    int nleft = len;
    int nread = 0;
    char * pbuf = buf;

    while(nread < len)
    {
        int ret = read(fd, pbuf, nleft);
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }else if(ret == 0)
        {
            return ret;
        }
        nleft -= ret;
        nread += ret;
        pbuf += ret;
    }
    if(nread > len)
        return -1;
    return nread;
}
size_t nwrite(int fd,char * buf, ssize_t len)
{
    int nleft = len;
    int nwritten = 0;
    char * pbuf = buf;

    while(nwritten < len)
    {
        int ret = write(fd, pbuf, nleft);
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;//this have some trouble, it needs run from head of thewhile
            return -1;
        }else if(ret == 0)
                  continue;
        nleft -= ret;
        nwritten += ret;
        pbuf += ret;
    }
    if(nwritten > len)
        return -1;
    return nwritten;
}

ssize_t readline(int fd, char * recvbuf, size_t maxsize)
{
    int count;
    for(;;)
    {
        if((count = recv(fd, recvbuf, maxsize, MSG_PEEK)) < 0)
        {
            if(errno == EINTR)
                continue;
            return count;
        } else if(count == 0)
                   return count;

        int i;
        for(i=0; i<count; i++)
        {
             if('\n' == recvbuf[i])
             {
                int ret = nread(fd, recvbuf, count);
                if(ret < 0)
                    return ret;
                return ret;
             }
        }
     }
}


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
        int ret = nwrite(sock_fd, sendbuf, strlen(sendbuf));
        if(ret < 0)
        {
            close(sock_fd);
            ERR_EXIT("write");
        }
        memset(recvbuf, 0, sizeof(recvbuf));
        ret = readline(sock_fd, recvbuf, sizeof(recvbuf));
        if(ret < 0)
        {
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
