#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define ERR_EXIT(m) \
        {  \
            perror(m); \
            exit(EXIT_FAILURE); \
        }while(0)

int main(void)
{
      int sock_fd;	
      if((sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
          ERR_EXIT("socket");
      struct sockaddr_in srv_addr;
      memset(&srv_addr, 0, sizeof(struct sockaddr_in));
      srv_addr.sin_family = AF_INET;
      srv_addr.sin_port = htons(3333);
      srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
      int conn_fd;
      if((connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr))) < 0)
          ERR_EXIT("conncet");
      char recvbuf[1024] = {0};
      char sendbuf[1024] = {0};
      while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL){
   
          write(sock_fd, sendbuf, strlen(sendbuf));
          memset(recvbuf, 0, sizeof(recvbuf));
          int ret = read(sock_fd, recvbuf, sizeof(recvbuf));
          if(ret == 0){
 		printf("server is closed");
		break;
          }
	  if(ret < 0)
	  {
	  	if(errno == EINTR)
                    continue;
		ERR_EXIT("read");	
     	  }
          fputs(recvbuf, stdout);
      }
      close(sock_fd);
      return 0;
}
