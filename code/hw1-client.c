/*

建立UDP客户端和UDP服务器，客户端发送信息到服务器，服务器接收信息并打印

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
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
/*	if(connect(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0)
	{
		perror("connect error!\n");
		exit(1);
	}*/

	/*步骤3: 和服务器端进行双向通信*/
	char buffer[1024] = "hello iotek!";
	memset(buffer, 0, sizeof(buffer));

	while(1)
	{
		printf("Plz input the message you want to send to server:");
		fgets(buffer,100,stdin);
		buffer[strlen(buffer)-1]='\0';

		printf("The message you input: %s\n",buffer);
		//向服务器发送数据报文
		if(sendto(sockfd, buffer, sizeof(buffer), 0,(struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			perror("sendto error!\n");
			exit(1);
		}else
		{//接收服务器端发送的数据报文
			memset(buffer, 0, sizeof(buffer));
			if(recv(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				perror("recv error!\n");
				exit(1);
			}else
			{
				printf("%s\n",buffer);
			}
		}
	}
	close(sockfd);

	return 0;
}
