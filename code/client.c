/*

创建一个TCP的客户端与TCP的服务器，服务器采用多进程并发结构，客户端可以一直给服务器发送数据，直到输入exit退出

此程序为客户端   client

Author : Renleilei
Date   : 2016-12-8
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

int main(int argc,char **argv)
{
	if(argc < 3)
	{
		printf("usage: %s ip port\n", argv[0]);
		exit(1);
	}

	/*步骤1: 创建socket */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
	if((size = read(sockfd, buffer, sizeof(buffer))) < 0)
	{
		perror("read error!\n");
	}
	if(write(STDOUT_FILENO, buffer, size) != size)
	{
		perror("write error!\n");
	}
	/*步骤4: 关闭socket*/
	close(sockfd);

	return 0;
}
