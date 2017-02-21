/*

创建一个TCP的客户端与TCP的服务器，服务器采用多进程并发结构，客户端可以一直给服务器发送数据，直到输入exit退出

此程序为客户端   client

客户端需具备功能:
1:新用户注册；(账号信息需发送数据包到服务器，服务器用链表存储)
2:用户登录；(用户发送登录的账号信息至服务器，匹配后允许进入)
3:心跳机制；(用户处于登录状态的情况下每3秒发送一个心跳数据包向服务器)
4:聊天功能; (用户可以查看当前在线用户并向一个用户发送信息)
5:命令功能; (用户可以向服务器发送命令，等待服务器返回执行结果并打印)

Author : Renleilei
Date   : 2016-12-10
Version: 1.0

*/


#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "view.h"
#include <unistd.h>
#include <signal.h>
#include "client.h"


void sig_handler(int signo);

int main(int argc,char **argv)
{
	if(argc < 3)
	{
		printf("usage: %s ip port\n", argv[0]);
		exit(1);
	}

	/*步骤1: 创建socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket error!\n");
		exit(1);
	}

	//往serverAddr中填入ip\port和地址族类型(ipv4)
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));

	//将ip地址转换为网络字节序后填入serveraddr中
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr.s_addr);

	/*步骤2: 客户端调用connect函数连接到服务器端*/
	if(connect(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0)
	{
		perror("connect error!\n");
		exit(1);
	}

	/*步骤3: 调用IO函数(read/write)和服务器端进行双向通信*/
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	size_t size;

	if(signal(SIGINT, sig_handler) == SIG_ERR)
	{
		perror("signal sigint error!");
		close(sockfd);
		exit(1);
	}

	mainMenuCtl(sockfd);

	/*步骤4: 关闭socket*/
	close(sockfd);

	return 0;
}

void sig_handler(int signo)
{
	if(signo == SIGINT)
	{
		printf("Client shut down!\n");
		close(sockfd);
		exit(1);
	}
}
