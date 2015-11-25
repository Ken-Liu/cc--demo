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
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};
    while(1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = nread(conn, recvbuf, sizeof(recvbuf));
        if(ret < 0)
        {
            close(sock_fd);
            close(conn);
            ERR_EXIT("nread");
        }else if(ret == 0)
        {
            printf("Client is closed");
            break;
        }
        fputs(recvbuf, stdout);
        ret = nwrite(conn, recvbuf, sizeof(recvbuf));
        if(ret < 0)
        {
            close(conn);
            close(sock_fd);
            ERR_EXIT("nwrite");
        }
    }
    close(sock_fd);
    close(conn);	        
}
