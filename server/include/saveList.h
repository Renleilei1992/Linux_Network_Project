/*

服务器控制函数头文件

*/

#ifndef _SAVELIST_H
#define _SAVELIST_H

typedef struct protocol{
    int type;   		//数据包类型
    char userName[30];  //用户名
    char userPwd[30];   //用户密码
	char cmd[30];		//用户发送命令
	char heart[30];		//心跳数据
	char chatName[30];  //聊天对象用户名
    char data[1024];    //数据内容
	char exit[10];		//退出指令
}Protocol;

typedef struct userInformation  		//存储在文件中的静态链表，保存用户信息
{
	char user_name[30];
	char user_pwd[30];
	char chat_name[30];
	char user_data[1024];
    struct userInformation *next;
}userInfo_t;

typedef struct userOnline  								 //动态链表，用于查询在线用户
{
    char userOL_name[30];   							//在线用户名
    char userIP[30];        							//在线用户IP
    int user_sockfd;        							//在线用户套接字
    struct userOnline *next;
}userOL_t;

struct list								//传参数用结构体，线程操作函数只能传一个参数
{
	char name[30];
	int fdOfList;
	userInfo_t *node;
	userOL_t *ol_node;
};

struct chat
{
	char launch_name[30];
	char chat_msg[1024];
};


userInfo_t* createList(int size);						//静态存储链表创建
			
userOL_t* create_ol_list(int size);						//动态链表创建

void userInfoListSave(userInfo_t *head);				//静态链表存储

void readList(userInfo_t *head);						//静态链表文件读取

void addTail(userInfo_t *head,userInfo_t *newNode);		//静态链表尾插法

void addTail_ol(userOL_t *head,userOL_t *newNode);		//动态链表尾插法


#endif
