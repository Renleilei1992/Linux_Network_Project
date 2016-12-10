/*

建立UDP客户端和UDP服务器，客户端发送信息到服务器，服务器打印出结果

此程序为服务器端   server

Author : Renleilei
Date   : 2016-12-8
Version: 1.0

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>

int sockfd;	//定义一个全局套接字文件描述符变量

//对信号进行处理，当ctrl+c输入的时候会终止程序
void sig_handler(int signo)
{
	if(signo == SIGINT)
	{
		printf("sever close\n");
		close(sockfd);
		exit(1);
	}
}

//输出连接的客户端的相关信息
void out_addr(struct sockaddr_in *clientAddr)
{
	//将端口获取的网络字节序转换为主机字节序
	int port = ntohs(clientAddr->sin_port);
	char ip[16];
	memset(ip, 0, sizeof(ip));
	//将ip地址从网络字节序转换为点分十进制
	inet_ntop(AF_INET, &clientAddr->sin_addr.s_addr, ip, sizeof(ip));
	printf("client: %s(%d) connected\n", ip, port);
}

//对客户端的fd进程操作，打印客户端信息
void do_service()
{
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);
	char rcv[1024];
	size_t size = sizeof(rcv);
	memset(rcv, 0, size);
//	recv(fd, rcv,sizeof(rcv),0);
	if(recvfrom(sockfd, rcv, size, 0,(struct sockaddr*)&clientAddr, &len) < 0)
	{
		perror("recvfrom error!\n");
	}else
	{
		out_addr(&clientAddr);
		printf("client send info: %s\n",rcv);
		sendto(sockfd, "Received! :)",12,0,(struct sockaddr*)&clientAddr, len);
		printf("Send back success!\n");
	}
}

int main(int argc,char **argv)
{
	if(argc < 2)
	{
		printf("usage: %s #port\n",argv[0]);
		exit(1);
	}
	if(signal(SIGINT, sig_handler) == SIG_ERR)
	{
		perror("signal sigint error!\n");
		exit(1);
	}

	/*步骤1:创建socket套接字
	 *socket创建在内核中，是一个结构体
	 *AF_INET: IPV4
	 *SOCK_DGRAM: UDP协议
	 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		perror("socket error!\n");
		exit(1);
	}
	int ret;
	int opt = 1;
	//设置套接字选项
	if((ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) <0 )
	{
		perror("setsocket error!\n");
		exit(1);
	}

	/*
	 *步骤2: 调用bind函数将socket和地址包括ip\port进行绑定
	 */
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	//向地址中填入ip\port\internet地址族类型
	serverAddr.sin_family = AF_INET;	//IPV4
	serverAddr.sin_port = htons(atoi(argv[1]));	//将主机字节序转化为网络字节

	//指定ip地址192.168.0.10?? 如果监听所有服务器上的ip，使用 INADDR_ANY
	//#define INADDR_ANY (uint32_t 0x00000000)
	serverAddr.sin_addr.s_addr = INADDR_ANY;//sin_addr中成员为s_addr

	if(bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind error!\n");
		exit(1);
	}

	/*
	 *步骤3: 和客户端进行双向的数据通信
	 */
	while(1)
	{
		do_service();
	}
}


