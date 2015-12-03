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
typedef  struct package{
    unsigned int len;
    char buf[1024];
} TYPE_PKG;

size_t nread(int fd, TYPE_PKG * recvb)
{
    char * buf = recvb->buf;
    for(;;){
        int ret = read(fd, &(recvb->len), 4);
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }else if(ret < 4)
        {
            return ret;
        }else{
             break;
        }
    }
    ssize_t len = recvb->len;
    memset(buf, 0, 1024);
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

size_t nwrite(int fd,TYPE_PKG *sendb)
{
    char * buf = sendb->buf;
    ssize_t len = sendb->len;
    int nleft = len;
    int nwritten = 0;
    char * pbuf = buf;
 
    for(;;)
    {
        int ret = write(fd, &len, 4);
        if(ret < 0)
        {
            if(errno == EINTR)
                continue;
            return ret;
        }else if(ret == 0)
           continue;
        else break;
    }
   
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
    TYPE_PKG recvpkg ;
    TYPE_PKG sendpkg ;
    memset(&recvpkg, 0, sizeof(recvpkg));
    memset(&sendpkg, 0, sizeof(sendpkg));
    while(fgets(sendpkg.buf, sizeof(sendpkg.buf), stdin) != NULL)
    {
        sendpkg.len = strlen(sendpkg.buf);
        int ret = nwrite(sock_fd, &sendpkg);
        if(ret < 0)
        {
            close(sock_fd);
            ERR_EXIT("write");
        }
        memset(&recvpkg, 0, sizeof(recvpkg));
        ret = nread(sock_fd, &recvpkg);
        if(ret < 0)
        {
            close(sock_fd);
            ERR_EXIT("read");
        }else if(ret == 0)
        {
            printf("server is close");
            break;
        }
        fputs(recvpkg.buf, stdout);
    }
    close(sock_fd);
}
