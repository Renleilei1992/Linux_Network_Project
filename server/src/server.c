/*

创建一个TCP的服务器，服务器采用多进程并发结构

此程序为服务器端   server

服务器功能: 
1.保存客户端用户注册信息(链表)，使用文本形式
2.支持多客户端连接
3.支持执行客户端发送的shell命令:ls -l , pwd , ipcs , ifconfig
4.本地链表，客户端ip、连接时间、用户名、socket
5.在线动态链表，客户端登录插入结点，客户端退出删除该结点

Author : Renleilei
Date   : 2016-12-10
Version: 1.0

*/

#include <fcntl.h>
#include <sys/stat.h>
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
#include <pthread.h>
#include <errno.h>
#include "saveList.h"
#include <malloc.h>

int sockfd;	
pthread_mutex_t mutex;

void sig_handler(int signo);
void out_addr(struct sockaddr_in *clientAddr);
//void do_service(int fd);
void* read_fun(void* p);

int createNewUser(userInfo_t *head,Protocol *rcvCREATE);
int	login(userInfo_t *head,Protocol *rcvLOG,userOL_t *head_ol,int fd_login);

int judge(userInfo_t *head,int val);
int sendBack(int fdback,int val);		//登录函数返回客户端信息
int sendBackCreate(int fdback,int val); //创建函数返回客户端信息

int excuteCMD(Protocol *rcvCMD,int backfd,userOL_t *ol_head);	//接收并执行客户端命令

userInfo_t *searchNameInList(userInfo_t *head,char *searchName);

int	heartBeat_handler(int fdHeart,Protocol *rcvheart,int count,char *name_client);//心跳数据包处理函数

static int rcvExit(Protocol *exitRcv);		//客户端退出数据包处理函数

//单人聊天函数
int soloChat(Protocol *rcvChat,userOL_t *head_ol_list,char *nameSelf,int chat_fd);

void printList(userInfo_t *head);
void printList_ol(userOL_t *head);

#define TYPE_EXIT 0		//退出数据包
#define TYPE_LOGIN 1	//登录数据包
#define TYPE_REG 2		//注册数据包
#define TYPE_MSG 3		//消息数据包
#define TYPE_HEART 4	//心跳数据包
#define TYPE_CMD 5		//命令数据包
#define TYPE_OK 6		//正常数据包
#define TYPE_ERROR 7	//出错数据包
#define TYPE_CHECK 8	//检查数据包
#define TYPE_CHAT 9		//聊天数据包
#define TYPE_READ 10	//在线用户数据包

int main(int argc,char **argv)
{
	//打开配置文件,在提供配置文件的时候不需输入端口号参数
	FILE *fp;
	char ip_server[30],port_server[30];
	fp = fopen("src/configure.txt","r");
	if(fp == NULL)
	{
		perror("文件打开失败! fopen error");
		exit(1);
	}else
	{
		fseek(fp,3,SEEK_SET);	//文件寻址到冒号，取地址
		fgets(ip_server, 20, fp);
		fseek(fp,18,SEEK_SET);	//文件指针移动到第二行冒号，取端口
		fgets(port_server, 20, fp);
		printf("IP of Server: %s",ip_server);
		printf("Port of Server: %s",port_server);
//		printf("Port of Server: %d",atoi(port_server));	//端口号字符转整型
		fclose(fp);
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

	int ret;
	int opt = 1;	//将端口设置为可以重复使用，判别依据是套接字
	if((ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0)
	{
		perror("set reuse socket error!");
		printf("server will shut down!");
		exit(1);
	}

	/*
	 *步骤2: 调用bind函数将socket和地址包括ip\port进行绑定
	 */
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	//向地址中填入ip\port\internet地址族类型
	serverAddr.sin_family = AF_INET;	//IPV4
	serverAddr.sin_port = htons(atoi(port_server));	//将主机字节序转化为网络字节

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

	userInfo_t *userList = createList(sizeof(userInfo_t));
	printf("链表创建成功!%p\n",userList);
	readList(userList);
	printf("链表读取成功!%p\n",userList);

	userOL_t *ol_list = create_ol_list(sizeof(userOL_t));
	printf("在线链表创建成功!%p\n",ol_list);

//	printList(userList);	//打印已注册的所有用户信息

	while(1)
	{
		int flag = accept(sockfd,(struct sockaddr*)&clientAddr,&clientAddr_len);

		if(flag < 0)
		{
			perror("accept error!\n");
			continue;
		}else
		{
			struct list *structPoint;
			structPoint = (struct list*)calloc(1,sizeof(struct list));
			structPoint->fdOfList = flag;
			structPoint->node = userList;
			structPoint->ol_node = ol_list;
			strncpy(structPoint->name,"client",sizeof("client"));
		
			out_addr(&clientAddr);
			pthread_t id;
			pthread_attr_t attr;
			pthread_attr_init(&attr);//设置线程分离
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
			printf("准备创建线程! fd:%d\n",structPoint->fdOfList);
			pthread_create(&id,&attr,(void *)read_fun,(void *)structPoint);	//创建线程
			printf("线程创建结束! 返回fd:%d\n",structPoint->fdOfList);
			sleep(1);	//必须要有等待，否则read_fun将无法接收到结构体指针
//			free(structPoint);
		}
	}
	close(sockfd);

	return 0;
}

//线程操作函数，对连接的客户端进行收发信息操作
void* read_fun(void* p)
{
	int heartCount = 0;						//心跳计数全局变量(使用处在580行附近)
	struct list *pp = (struct list*)p;		//强转void指针为list指针

	int fd1 = pp->fdOfList;			
	char *nameClient = pp->name;
	userOL_t *ol_head = pp->ol_node;
	userInfo_t *head = pp->node;	

	Protocol *rcv;
	rcv = (struct protocol *)calloc(1,sizeof(struct protocol));
	pthread_mutex_init(&mutex,NULL);


	while(1)
	{
		int retval1,retval2,retval3 = 0;
		int cnt = 0;
		printf("fdread = %d\n",fd1);
		cnt = read(fd1,(void*)rcv,sizeof(struct protocol));	//读取客户端发送的信息
		if(cnt > 0)
		{
//			printf("成功读取！\n");

			switch(rcv->type)
			{
			case TYPE_EXIT:						//退出
				pthread_mutex_lock(&mutex);
				rcvExit(rcv);			//接收客户端退出信息
				pthread_mutex_unlock(&mutex);	//解锁
				strncpy(nameClient,"client",sizeof("client"));
				break;
			case TYPE_LOGIN:					//登录
				retval3 = login(head,rcv,ol_head,fd1);//根据密码与链表匹配得到返回值
				sendBack(fd1,retval3);		//根据login函数返回值向客户端返回信息
				if(retval3 == 666){
					strncpy(nameClient,rcv->userName,sizeof(rcv->userName));
				}
				break;
			case TYPE_REG:						//注册
				retval1 = createNewUser(head,rcv);//将结构体存入链表的最尾端节点
				retval2 = judge(head,retval1);	//判断是否保存链表
				sendBackCreate(fd1,retval2);
				break;
			case TYPE_MSG:						//单人聊天消息数据包
				soloChat(rcv,ol_head,nameClient,fd1);
				break;
			case TYPE_HEART:					//心跳数据包
/*				int fp = open("src/log.txt", O_WRONLY | O_TRUNC);	//打开日志文件
				int x = dup(1);
				int y = dup(2);
				dup2(fp,1);
				dup2(fp,2);*/
				heartCount = heartBeat_handler(fd1,rcv,heartCount,nameClient);
				heartCount++;
/*				dup2(x,1);
				dup2(y,2);*/
				break;
			case TYPE_CMD:						//命令数据包
				excuteCMD(rcv,fd1,ol_head); //接收客户端发送的命令执行并返回
				break;
			case TYPE_OK:						//正常数据包

				break;
			case TYPE_ERROR:					//出错数据包

				break;
			case TYPE_CHECK:					//检查数据包

				break;
			case TYPE_CHAT:						//群聊天数据包

				break;
			case TYPE_READ:						//xx用户数据包

				break;
			default:
				printf("未知类型! unknown type!\n");
				break;
			}
		}else if(0 == cnt)
		{
			printf("客户端网络连接已断开！\n");
			if(heartCount < 5)
			{
				heartCount = 4;
			}
			break;
		}else if(cnt < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}else
			{
				perror("读取失败！");
//				break;
			}
		}
	}
	while(1)
	{
		if(heartCount >= 10)
		{
			pthread_exit(0);
			break;
		}else
		{
			sleep(3);
			printf("heartCount == %d\n",heartCount);
			heartCount++;
		}
	}

	free(rcv);
	return 0;
}

int login(userInfo_t *head,Protocol *rcvLOG,userOL_t *head_ol,int fd_login)
{
	userInfo_t *findNode;

	findNode = searchNameInList(head,rcvLOG->userName);//账户名查重

	userOL_t *newNode;
	newNode = (userOL_t *)calloc(1,sizeof(userOL_t));

	if(findNode == NULL)
	{
		printf("无此用户!\n");
		free(newNode);
		return 555;
	}else
	{
		printf("此用户存在!\n");
        if(strncmp(findNode->user_pwd,rcvLOG->userPwd,30) == 0)
		{
			printf("密码正确,请稍等!\n");
			strncpy(newNode->userOL_name,rcvLOG->userName,30);
			newNode->user_sockfd = fd_login;
			addTail_ol(head_ol,newNode);
			usleep(100000);
//			printList_ol(head_ol);
			return 666;
		}else
		{
			printf("密码错误,请重新登录!\n");
			free(newNode);
			return 444;
		}
	}
	return 445;
}

int sendBack(int fdback,int val)		//根据login函数返回值向客户端返回信息
{
	char backData[1];
	bzero(backData,sizeof(backData));
	if(val == 666)
	{
		printf("密码匹配,允许客户端登录,正在发送消息回客户端!\n");
		strncpy(backData,"1",1);
		write(fdback,backData,1);
		printf("sendback success!\n");
		return 11;
	}else if(val == 444)
	{
		printf("密码不匹配,不允许客户端登录,正在发送消息回客户端!\n");
		strncpy(backData,"2",1);
		write(fdback,backData,1);
		printf("sendback success!\n");
		return 12;
	}else if(val == 555)
	{
		printf("用户名不存在,不允许客户端登录,正在发送消息回客户端!\n");
		strncpy(backData,"3",1);
		write(fdback,backData,1);
		printf("sendback success!\n");
		return 13;
	}else
	{
		printf("无法识别!\n");
		return 10;
	}
	return 10;
}

//判断函数,根据新建用户函数返回值判断是否需要保存该结点
int judge(userInfo_t *head,int val)
{
	if(head == NULL)
	{
		perror("head is NULL");
		return 55;
	}
	//根据返回值判断是否需要保存节点
	if(val == 99)
	{
		printf("创建失败,该用户名已存在!\n");
		return 56;
	}else if(val == 66)
	{
		printf("新用户创建成功!\n");
		userInfoListSave(head);
		printf("保存成功!\n");
		return 57;
	}

	return 58;
}

int sendBackCreate(int fdback,int val)//根据create函数返回值向客户端返回信息
{
	char backData[1];
	bzero(backData,sizeof(backData));
	if(val == 57)
	{
		printf("新用户创建成功,正在发送消息回客户端!\n");
		strncpy(backData,"1",1);
		write(fdback,backData,1);
		printf("sendback success!\n");
		return 11;
	}else if(val == 56)
	{
		printf("此用户名已被占用,正在发送消息回客户端!\n");
		strncpy(backData,"2",1);
		write(fdback,backData,1);
		printf("sendback success!\n");
		return 12;
	}else
	{
		printf("无法识别!\n");
		return 10;
	}
	return 10;
}

//根据注册信息在客户端创建新用户,使用尾插法
int createNewUser(userInfo_t *head,Protocol *rcvCREATE)
{
	userInfo_t *newNode;
	newNode = (userInfo_t *)calloc(1,sizeof(userInfo_t));
	userInfo_t *findNode;
	findNode = (userInfo_t *)calloc(1,sizeof(userInfo_t));
	printf("userList: %p\n",head);
//	userInfo_t *last = head->next;	//若head->next为NULL则段错误
	userInfo_t *last = head;
	
	if(last == NULL)
	{
		perror("head node is NULL");
		return 99;
	}

	findNode = searchNameInList(head,rcvCREATE->userName);//账户名查重

	if(findNode == NULL)
	{
		printf("可以创建新用户!\n");
	}else
	{
		printf("该用户名: %s 已被使用!\n",rcvCREATE->userName);
		return 99;
	}


	while(last->next != NULL)
	{
		last = last->next;
	}

	strncpy(newNode->user_name,rcvCREATE->userName,30);	
	strncpy(newNode->user_pwd,rcvCREATE->userPwd,30);	
	printf("newNode->user_name: %s\n",newNode->user_name);
	printf("newNode->user_pwd: %s\n",newNode->user_pwd);
	last->next = newNode;
	newNode->next = NULL;	//将新结点的next置空，即新结点作为尾巴结点

	return 66;
}


int excuteCMD(Protocol *rcvCMD,int backfd,userOL_t *ol_head)	//执行客户端命令函数
{
	if(rcvCMD == NULL)
	{
		printf("接收到的命令为 NULL");
		return 20;
	}else
	{
//		printf("接收到的命令为: %s",rcvCMD->cmd);
		int fpCMD = open("src/CMD.txt", O_WRONLY | O_TRUNC);//擦除文件内容写入打印信息
	
		int x =	dup(1);				//备份屏幕标准输出和标准错误输出
		int y =	dup(2);
		dup2(fpCMD,1);				//将shell命令执行结果重定向到文件中
		dup2(fpCMD,2);
		if(strcmp("check",rcvCMD->cmd) == 0)
		{
			printList_ol(ol_head);
		}else
		{
			system(rcvCMD->cmd);
		}
		close(fpCMD);
		
		dup2(x,1);							//重定向恢复
		dup2(y,2);
		
		FILE *fp;
		fp = fopen("src/CMD.txt","r");		//以只读打开文件
		char msgCMD[1024];					//定义一个字符串
		int count = 0;
		do{									//将只读打开文件内容1024字节读出
			bzero(msgCMD,sizeof(msgCMD));
			count = fread(msgCMD,1024,1,fp);
			printf("\nsend back msg is writing!\n");
			write(backfd, msgCMD, 1024);	//将读出的文件内容写入backfd
			count--;
		}while((count+1));
		
		fclose(fp);
		return 21;
	}

	return 22;
}


//按照姓名查询链表中的信息,返回值是一个链表节点
userInfo_t *searchNameInList(userInfo_t *phead,char *searchName)
{
    userInfo_t *ret = NULL;
    ret = phead;
	
	if(ret == NULL)
	{
		perror("链表内容为空!");
		return NULL;
	}

	while(ret != NULL)
    {   
        if(strncmp(ret->user_name,searchName,30) == 0)
        {   
	    	return ret;
	    }   
	    ret = ret->next;
	}   
    return NULL;
}

//按照姓名查询动态链表中的信息,返回值是一个链表节点
userOL_t *searchName_olList(userOL_t *phead,char *searchName)
{
    userOL_t *ret = NULL;
    ret = phead;
	
	if(ret == NULL)
	{
		perror("链表内容为空!");
		return NULL;
	}

	while(ret != NULL)
    {   
        if(strncmp(ret->userOL_name,searchName,30) == 0)
        {   
	    	return ret;
	    }   
	    ret = ret->next;
	}   
    return NULL;
}

//单人聊天函数，传入数据结构体，动态链表头节点，当前用户名
int soloChat(Protocol *rcvChat,userOL_t *head_ol_list,char *nameSelf,int chat_fd)
{
	struct chat *Chat_content;
	Chat_content = malloc(sizeof(struct chat));

	if((rcvChat || head_ol_list) == 0)
	{
		perror("solo chat error");
		return 50;
	}

	userOL_t *chat_target = searchName_olList(head_ol_list, rcvChat->chatName);

	if(chat_target == NULL)
	{
		printf("您发起的聊天对象: %s 不在线!\n",rcvChat->chatName);
		return 50;
	}else if(chat_target->user_sockfd == chat_fd)
	{
		printf("您不能向自己发送信息!\n");
		return 50;
	}else
	{
		printf("%s 向 %s 发起聊天!\n",nameSelf,chat_target->userOL_name);
		printf("发送信息为: %s\n",rcvChat->data);
		strncpy(Chat_content->launch_name,nameSelf,30);
		strncpy(Chat_content->chat_msg,rcvChat->data,1024);

		write(chat_target->user_sockfd, Chat_content, sizeof(struct chat));
//		usleep(100000);
		return 51;
	}
	return 52;
}


//打印出静态链表所有结点的结构体内容
void printList(userInfo_t *head)
{
	userInfo_t *toPrint = head->next;
	while(toPrint != NULL)
	{
		printf("name: %s\n",toPrint->user_name);
		printf("pwrd: %s\n",toPrint->user_pwd);
		toPrint = toPrint->next;
	}
}

//打印出动态链表所有结点的结构体内容
void printList_ol(userOL_t *head)
{
	userOL_t *toPrint = head->next;
	while(toPrint != NULL)
	{
//		printf("userOL_name: %s\n",toPrint->userOL_name);
//		printf("user_sockfd: %d\n",toPrint->user_sockfd);
		printf("在线用户: %10s          ->sockfd: %d\n",
								toPrint->userOL_name,toPrint->user_sockfd);
		toPrint = toPrint->next;
	}
}


//对信号进行处理，当ctrl+c输入的时候会终止程序
void sig_handler(int signo)
{
	if(signo == SIGINT)
	{
		printf("Sever close!\n");
		close(sockfd);
		exit(1);
	}
}

int	heartBeat_handler(int fdHeart,Protocol *rcvheart,int count,char *name_client)
{
	if(count > 4)
	{
		count = 0;
	}

	if(NULL == rcvheart)
	{
		printf("心跳异常!\n");
		return count++;
	}else
	{
		if(strcmp("client",name_client) == 0)
		{
			printf("Client sockfd: %10d alive!  heartCount == %d %s",
									fdHeart,count,rcvheart->heart);
		}else
		{
			printf("Client name: %10s alive!  heartCount == %d %s",
									name_client,count,rcvheart->heart);
		}
		return count++;
	}

	return count;
}


static int rcvExit(Protocol *exitRcv)					//接收客户端退出信息
{
	if(strcmp("exit",exitRcv->exit) == 0)
	{
		printf("客户端已自行退出!\n");
		return 41;
	}
	return 40;
}

//服务器端显示当前连接客户端的相关信息,IP、端口、时间、套接字
void out_addr(struct sockaddr_in *clientAddr)
{
	//将端口获取的网络字节序转换为主机字节序
	int port = ntohs(clientAddr->sin_port);
	char ip[16];
	long t = time(0);
	char *s = ctime(&t);
//	size_t size = strlen(s)*sizeof(char);
//	int fp = open("src/connect_log.txt", O_WRONLY | O_TRUNC | O_APPEND);	//打开日志文件
	int fp = open("src/connect_log.txt", O_WRONLY | O_APPEND);	//打开日志文件
	int x = dup(1);
	int y = dup(2);
	dup2(fp,1);
	dup2(fp,2);

	memset(ip, 0, sizeof(ip));
	//将ip地址从网络字节序转换为点分十进制
	inet_ntop(AF_INET, &clientAddr->sin_addr.s_addr, ip, sizeof(ip));
	printf("Client IP: %s(%d)connected\ntime: %ssockfd: %d\n",
		   					ip, port, s, sockfd);
	dup2(x,1);
	dup2(y,2);
	printf("Client IP: %s(%d)connected\ntime: %ssockfd: %d\n",
		   					ip, port, s, sockfd);
}

