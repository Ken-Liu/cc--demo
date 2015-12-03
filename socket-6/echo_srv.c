#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#define ERR_EXIT(m) \
         do{        \
         perror(m); \
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
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(5566);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int on = 1;
    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");
    if(bind(sock_fd,(struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
	ERR_EXIT("bind");
    if(listen(sock_fd, SOMAXCONN) < 0)
	ERR_EXIT("listen");
    int conn;
    int len = sizeof(srv_addr);
    int client[FD_SETSIZE];
    int i;
    memset(client, -1, sizeof(client));
    fd_set allset;
    fd_set set;
    FD_ZERO(&set);
    FD_ZERO(&allset);
    FD_SET(sock_fd, &allset);
    set = allset;
    int maxfd = sock_fd + 1;
    while(1){
        set = allset;       
        int nready = select(maxfd, &set, NULL, NULL, NULL);
        if( nready < 0 )
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("select");
        }else if(nready == 0)
            continue;
        if(nready > 0)
        {
            if(FD_ISSET(sock_fd, &set))
            {
                if((conn = accept(sock_fd, (struct sockaddr *)&srv_addr, &len)) < 0)
                    ERR_EXIT("accept");
                if(conn >= maxfd)
                    maxfd = conn + 1;
                for(i=0; i<FD_SETSIZE; i++)
                {
                    if(client[i] == -1)
                    {
                        client[i] = conn;
                        FD_SET(conn, &allset);
                        break;
                    }
                }
                if(i >= FD_SETSIZE)
                {
                    printf("Too many conn\n");
                    continue;
                }
                if(nready == 0)
		    continue;
                    
            }
            TYPE_PKG recvbuf;
            memset(&recvbuf, 0, sizeof(recvbuf));
            for(i=0; i<FD_SETSIZE; i++)
            {
                if(FD_ISSET(client[i], &set))
                {
                   int ret = nread(client[i], &recvbuf);
                   if(ret < 0)
                   {
           	       if(errno == EINTR)
                           continue;
                       ERR_EXIT("nread"); 
                   }else if(ret == 0)
                   {
                       printf("client is closed\n");
                       FD_CLR(client[i], &allset);
                       close(client[i]);
                       client[i] = -1;
                       continue;
                   }
                   fputs(recvbuf.buf, stdout);
                   ret = nwrite(client[i], &recvbuf);
                   if(ret < 0)
                   {
                       if(errno == EINTR)
                           continue;
                       ERR_EXIT("nwrite");
                   }
                   
                }
            }
        }
    }
    close(sock_fd);
    return 0;	        
}