#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>


#define ERR_EXIT(m)            \
        {                      \
          perror(m);      \
          exit(EXIT_FAILURE); \
        }while(0)

int main(void)
{
     int sock;      
     
     if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
         ERR_EXIT("socket"); 
     unsigned short port = htons(3333);
     struct sockaddr_in srv_ip;
     memset(&srv_ip, 0, sizeof(struct sockaddr_in));
     int srv_addrlen = sizeof(struct sockaddr_in);
     srv_ip.sin_family = AF_INET;
     srv_ip.sin_port = port;
     srv_ip.sin_addr.s_addr = inet_addr("127.0.0.1");// htonl(INADDR_ANY);
     if(bind(sock, (struct sockaddr *)&srv_ip, srv_addrlen) < 0)
         ERR_EXIT("bind");
     if(listen(sock, SOMAXCONN) < 0)
         ERR_EXIT("listen");
     int conn;
     if((conn = accept(sock, (struct sockaddr *)&srv_ip, &srv_addrlen)) < 0)
         ERR_EXIT("accept");
     char recvbuf[1024];
     while(1){
         memset(recvbuf, 0, sizeof(recvbuf));
         int ret = read(conn, recvbuf, sizeof(recvbuf));
         if(ret == 0)
         {
	    printf("Client is closed");
            break;
         }
         if(ret < 0)
	 {
	    if(errno == EINTR)
	         continue;
            ERR_EXIT("read");
     	 }
         fputs(recvbuf,stdout);
         write(conn, recvbuf, strlen(recvbuf));
     }
     
     close(conn);
     close(sock);
     return 0;

}
