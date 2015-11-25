#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
int main(void)
{
   unsigned int x = 0x12345678;
   char * str_addr = "192.168.0.100";
   unsigned  char *p = (unsigned char *) &x;
   unsigned long y = 0;
   unsigned long int_addr = 0;
   unsigned char str_test_addr[16];
   struct in_addr test_addr;
   int a = 0;   
   printf("%0x,%0x,%0x,%0x\n",p[0],p[1],p[2],p[3]);
   y = htonl(x);
   p = (unsigned char *) &y;
   printf("%0x,%0x,%0x,%0x\n",p[0],p[1],p[2],p[3]);
   int_addr = inet_addr(str_addr);//当前是网络字节序
   test_addr.s_addr = int_addr;
   inet_ntop(AF_INET,&test_addr,str_test_addr,sizeof(str_test_addr));
   printf("%s",str_test_addr);
   return 0; 
}
