/*

创建一个TCP的客户端与TCP的服务器，客户端向服务器发送shell命令
服务器执行并返回给客户端

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

//对客户端的fd进程操作，此处为接收客户端的shell命令，并返回值
void do_service(int fd)
{
	long t = time(0);
	int i = 0; 
	char *s = ctime(&t);
	char rcv[100];
	size_t size = strlen(s) * sizeof(char);
	memset(rcv, 0, sizeof(rcv));
//	recv(fd, rcv,sizeof(rcv),0);
	if(read(fd, &rcv, size) != size)
	{
		perror("read error!\n");
	}
	printf("The message received: %s\n",rcv);
	if((i = system(rcv)) < 0)
	{
		printf("excute failed!\n");
	}
	write(fd, "The command received! :)",24);
	printf("Send back success!\n");
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
	 *SOCK_STREAM: TCP协议
	 */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket error!\n");
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
	 *步骤3: 调用listen函数启动监听(指定port监听)
	 *		 通知系统去接收来自客户端的连接请求
	 *		 (将接收到的客户端连接请求放置到队列中，队列长度可以指定)
	 */
	if(listen(sockfd, 10) < 0)
	{
		perror("listen error!\n");
		exit(1);
	}
	/*
	 *步骤4: 调用accept函数从队列中获得一个客户端的请求连接
	 *		 返回一个新的socket描述符
	 *		 第二个参数若传入了结构体，客户端的信息就能读取，可以置空NULL
	 *注意: 若没有客户端连接服务器，调用accept函数会阻塞
	 *		直到获取一个客户端连接
	 */
	struct sockaddr_in clientAddr;
	socklen_t clientAddr_len = sizeof(clientAddr);
//	int fd = accept(sockfd,(struct sockaddr*)&clientAddr,&clientAddr_len);
	while(1)
	{
		int fd = accept(sockfd,(struct sockaddr*)&clientAddr,&clientAddr_len);
		pid_t pid;
		pid = fork();
		if(pid == 0)
		{
			while(1)
			{	
				if(fd < 0)
				{
					perror("accept error!\n");
					continue;
				}else
				{
					sleep(1);
					out_addr(&clientAddr);
					do_service(fd);
				}
//				close(fd);
				char get[100] = {0};
				read(fd, get, sizeof(get));
			}
		}
	}
	close(sockfd);

	return 0;
}


