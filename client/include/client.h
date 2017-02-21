/*


此程序为客户端函数控制程序头文件


Author : Renleilei
Date   : 2016-12-10
Version: 1.0

*/

#ifndef _CLIENT_H
#define _CLIENT_H

typedef struct protocol
{
    int type;           //数据包类型
    char userName[30];  //用户名
    char userPwd[30];   //用户密码
	char cmd[30];		//用户发送命令
	char heart[30];		//心跳数据
	char chatName[30];  //聊天对象
    char data[1024];    //数据
	char exit[4];		//退出指令
}Protocol;

struct chat
{
	char launch_name[30];
	char chat_msg[1024];
};

int sockfd;

void mainMenuCtl(int sockfd);

void packageSend(void *p,int sockfd);

void createNewUser(int sockfd);

int login(int sockfd);

void userMenuCtl(int sockfd);

#endif
