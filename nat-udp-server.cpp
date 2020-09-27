#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
/*
 * 服务端获得客户端的外网ip和port, 并将客户端的消息原样发回给客户端
 */

#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(1);\
    }while(0)
 
/* 用来记录客户端发送过来的外网ip+port */
typedef struct{
    struct in_addr ip;
    int port;
}clientInfo;
 
#define DEFAULT_PORT 8888
int main(int argc, char*argv[])
{
    /* 一个客户端信息结构体数组，分别存放两个客户端的外网ip+port */
    clientInfo info[2];
    /* 作为心跳包需要接收的一个字节 */
    /* char ch; */ 
    char str[100] = {0};

    if (argc < 2){
        printf("Usage: %s <port>\n", argv[0]);
	return 0;
    }
    int listen_port = atoi(argv[1]);

    printf("nat-udp-server:%d start ...\n", listen_port);


 
    /* 创建udp server */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1)
        ERR_EXIT("SOCKET");
	
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    serveraddr.sin_port = htons(listen_port);
    serveraddr.sin_family = AF_INET;    
 
    int ret = bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(ret == -1)
        ERR_EXIT("BIND");
 
    /* 服务器接收客户端发来的消息并转发 */
    while(1)
    {
        bzero(info, sizeof(clientInfo)*2);
		
        /* 接收客户端的包并记录此链接的ip+port */
        socklen_t addrlen = sizeof(struct sockaddr_in);
        recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&serveraddr, &addrlen);

        memcpy(&info[0].ip, &serveraddr.sin_addr, sizeof(struct in_addr));
        info[0].port = serveraddr.sin_port;

        printf("Client IP:%s \tPort:%d(=%d)  :: creat link OK! str=%s\n", 
			   inet_ntoa(info[0].ip), ntohs(info[0].port), info[0].port, str);

 
        /* 向客户端回射信息 */
        serveraddr.sin_addr = info[0].ip;
        serveraddr.sin_port = info[0].port;
	str[strlen(str)] = '\0';
        printf("Send message to client:: IP:%s  \tPort:%d :: str=%s\n", 
              inet_ntoa(serveraddr.sin_addr), serveraddr.sin_port, str);
        sendto(sockfd, &str, strlen(str), 0, (struct sockaddr *)&serveraddr, addrlen);

	memset(str, 0, sizeof(str));

    }
    return 0;
}

