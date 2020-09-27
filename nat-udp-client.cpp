#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
 
/* 原理见服务器源程序 */
#define ERR_EXIT(m)\
    do{\
        perror(m); \
        exit(1);\
    }while(0)
 
typedef struct{
    struct in_addr ip;
    int port;
}clientInfo;

double tick(void)
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + 1E-6 * t.tv_usec;
}
 
/* 用于udp和服务器通信 */
void echo_ser(int sockfd, struct sockaddr* addr, socklen_t *len)
{   
    char send_buf[1024];
    char recv_buf[1024];
    int  pkt_cnt = 0;

    bzero(send_buf, sizeof(send_buf));
    bzero(recv_buf, sizeof(recv_buf));
    while(1) {
        sprintf(send_buf, "%s : %d", "pkt_cnt=", pkt_cnt);

        double st = tick();
        /* 发送数据 */
        sendto(sockfd, send_buf, strlen(send_buf), 0, addr, sizeof(struct sockaddr_in));


 
        /* 接收发来的数据 */
        recvfrom(sockfd, recv_buf, sizeof(recv_buf)-1, 0, addr, len);
        double dt = tick() - st;
        printf("RECV: %s , len = %d, Time-tick=%.3f sec\n", recv_buf, len, dt);
        recv_buf[strlen(recv_buf)] = '\0';


        bzero(send_buf, sizeof(send_buf));
        bzero(recv_buf, sizeof(recv_buf));
        pkt_cnt++;
        if (pkt_cnt >= 100){
            break;
        }
        sleep(1);
    }
}
 
int main(int argc, char* argv[]){
    if (argc < 3) {
       printf("Usage:%s <server-addr> <server-port>\n", argv[0]);
       exit (0);
    }
    printf("NAT-UDP client B start ...\n");
    
    char* server_addr = argv[1];
    int   server_port = atoi(argv[2]); 




    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1)
        ERR_EXIT("SOCKET");
	
    /* 向服务器发送心跳包的一个字节的数据 */
    char ch = 'a';
    /* char str[] = "abcdefgh"; */
    clientInfo info;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    bzero(&info, sizeof(info));
    struct sockaddr_in clientaddr, serveraddr;
    /* 客户端自身的ip+port */
    /* 服务器的信息 */
    memset(&clientaddr, 0, sizeof(clientaddr));

    /* 实际情况下为一个已知的外网的服务器port */
    serveraddr.sin_port = htons(server_port);
    /* 实际情况下为一个已知的外网的服务器ip,这里仅用本地ip填充，下面这行的ip自己换成已知的外网服务器的ip */
    serveraddr.sin_addr.s_addr = inet_addr(server_addr);   
    serveraddr.sin_family = AF_INET;
 
    /* 向服务器S发送数据包 */
    sendto(sockfd, &ch, sizeof(ch), 0, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in));
	
    printf("send success\n");
	char str[100] = {0};
    recvfrom(sockfd, &str, sizeof(str), 0, (struct sockaddr *)&serveraddr, &addrlen);
	str[strlen(str)] = '\0';
	
    printf("Server info:%s\n", str);
    echo_ser(sockfd, (struct sockaddr *)&serveraddr, &addrlen);
    close(sockfd);
    return 0;
}
