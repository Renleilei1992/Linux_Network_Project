/*


此程序为客户端函数控制程序


Author : Renleilei
Date   : 2016-12-10
Version: 1.0

*/

#include <time.h>
#include "client.h"
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
#include <termios.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

static void rcvBackData(int fdback,int sockfd);//接收服务器登录信息

static void rcvBackDataCreate(int fdback);//接收服务器创建用户的返回数据

void sendCommand(int sendfd);			//向服务器发送命令	
static void rcvCommandBack(int rcvfd);	//接收服务器执行命令后的返回值并打印

void heartBeat(int signo);		//心跳数据包打包并发送,使用信号捕捉方式
void sendExit(int exitfd);		//退出指令数据包发送

void checkOL(int checkfd);		//向服务器发送"check"命令查询服务器在线用户	
void chat_to(int chatfd);		//发送聊天数据包
static void chat_rcv(void *p);//登陆之后接收服务器发送的聊天信息(线程)

static int getch();

static int getch()   //定义一个不需要切换至控制台的getch函数
{
    int c = 0; 
    struct termios org_opts, new_opts;
    int res=0;
    res=tcgetattr(STDIN_FILENO, &org_opts);
    assert(res==0);
    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL); 
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    c=getchar();
    res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res==0);

    return c;
}


//客户端主菜单控制函数
void mainMenuCtl(int sockfd)
{
	char choice;
	int retval = 0;

	while(1)
	{
		mainMenu();
		scanf("%c",&choice);
		while(getchar() != '\n');

		switch(choice)
		{
		case '1':		//客户端用户登录
			retval = login(sockfd);
			if(retval){
			printf("\n正在校验账户密码.....\n");
			rcvBackData(sockfd,sockfd);}	//接收服务器返回信息判断是否能登录
			break;
		case '2':		//客户端用户注册
			createNewUser(sockfd);
			printf("\n正在向服务器发送请求.....");
			rcvBackDataCreate(sockfd);
			break;
		case '3':		//退出
			close(sockfd);
			printf("%30s\033[1;32m%s\033[0m\n"," ","=====谢谢使用!=====");
			return;
		default:
			printf("您输入的选项有误，请重新输入或退出!\n");
			break;
		}
	}
}

void userMenuCtl(int sockfd)	//用户登录之后的主控函数
{
	char choice;

	signal(SIGALRM, heartBeat);	//信号捕获函数，捕捉心跳函数
	alarm(3);
	

	while(1)
	{
		userMenu();
		scanf("%c",&choice);
		while(getchar() != '\n');

		switch(choice)
		{
		case '1'://在线好友列表(查看当前在线的用户)
			checkOL(sockfd);
			sleep(1);
			rcvCommandBack(sockfd);
			break;
		case '2'://在线单人聊天
			chat_to(sockfd);		//向服务器发送聊天数据包并创建线程接收
			break;
		case '3':
			sendCommand(sockfd);	//向服务器发送命令	
			sleep(1);
			rcvCommandBack(sockfd); //接收服务器执行命令后的结果并打印
			break;
		case '4':
			sendExit(sockfd);	//向服务器发送退出客户端指令
			return;
		default:
			printf("您输入的选项有误，请重新选择或退出!\n");
			break;
		}
	}
}

int login(int sockfd)	//客户端登录函数
{
	Protocol *userAccount;
	userAccount = (struct protocol*)calloc(1,sizeof(struct protocol));
	int i = 0;
	char pd[128] = {0};
	
	printf("%30s\033[1;32m%s\033[0m\n"," ","=====登录界面=====");
	userAccount->type = 1;
	printf("请输入账户名:");
	fgets(userAccount->userName,30,stdin);
	userAccount->userName[strlen(userAccount->userName)-1] = '\0';

	printf("请输入密码:");
	for(i=0;;i++)
	{
		pd[i] = getch();
		if(pd[i] == '\n')
		{
			pd[i] = '\0';
			break;
		}
		if(pd[i] == 127)
		{
			printf("\b \b");
			i = i-2;
		}else
		{
			printf("*");
		}
		if(i < 0)
		{
			pd[0]='\0';
		}
	}
	printf("\n您输入的密码为: %s",pd);
	strcpy(userAccount->userPwd,pd);

	srand((int)time(NULL));
	int p = rand()%60+1;
	int q = rand()%40+1;
	int inputAnswer = 0;

	printf("\n%d+%d = ?\n",p,q);
	printf("请输入计算结果:");
	scanf("%d",&inputAnswer);
	while(getchar() != '\n');
	if(inputAnswer != (p+q))
	{
		printf("您输入的计算结果有误!");
		return 0;
	}

	packageSend(userAccount,sockfd);//将结构体数据打包为数据包发送到服务器

	free(userAccount);
	return 1;
}

void checkOL(int checkfd)	//向服务器发送"check"命令查询服务器在线用户	
{
	Protocol *cmdCheck;
	cmdCheck = (struct protocol*)calloc(1,sizeof(struct protocol));

	cmdCheck->type = 5; 
	strncpy(cmdCheck->cmd,"check", sizeof("check"));

//	printf("您需要执行的命令: %s\n",cmdCheck->cmd);
	packageSend(cmdCheck,checkfd);

	free(cmdCheck);
}

void sendCommand(int sendfd)	//向服务器发送命令	
{
	Protocol *cmdSend;
	cmdSend = (struct protocol*)calloc(1,sizeof(struct protocol));

	cmdSend->type =	5; 
	printf("请输入您需要执行的命令:");
	fgets(cmdSend->cmd, 30, stdin);

//	printf("您需要执行的命令: %s\n",cmdSend->cmd);

	packageSend(cmdSend,sendfd);

	free(cmdSend);
}

static void rcvCommandBack(int rcvfd)//接收服务器执行命令后的返回数据并打印
{
	char dataBack[1024];
	bzero(dataBack,sizeof(dataBack));
	
	int len = 0;

	int flags = fcntl(rcvfd, F_GETFL, 0);
	fcntl(rcvfd, F_SETFL, flags | O_NONBLOCK);
	while(0 != (len = recv(rcvfd,dataBack,sizeof(dataBack),0)))
	{
		if(len < 0)
		{
//			printf("Recv Data From Sever Failed!\n");
			break;
		}

		printf("%s",dataBack);
		fflush(stdout);
	}

/*	while(1)	
	{
		bzero(dataBack,sizeof(dataBack));
		val = read(rcvfd,dataBack,sizeof(dataBack));
		printf("%s",dataBack);
		fflush(stdout);
//		printf("val:%d\n",val);
		if(val <= 0 )
		{
			break;
		}
		usleep(100000);
	}*/
}

static void rcvBackData(int fdback,int sockfd)//登录函数接收服务器返回信息判断是否能登录
{
	char dataBack[1];
	bzero(dataBack,sizeof(dataBack));
	
	if(read(fdback,dataBack,1) < 0)
	{
		printf("nothing back!\n");
		return;
	}else
	{
		printf("reading....\n");
//		sleep(1);
		printf("Backmsg: %d\n",atoi(dataBack));
		if(atoi(dataBack) == 1)
		{	
			system("clear");
			printf("%30s\033[1;32m%s\033[0m\n"," ","【欢迎尊贵的会员】");
			userMenuCtl(sockfd);
			return;
		}else if(atoi(dataBack) == 2)
		{
			printf("===您输入的密码有误，请重新登录！===\n");
			return;
		}else if(atoi(dataBack) == 3)
		{
			printf("===账户不存在，请重新登录！===\n");	
			return;
		}else
		{
			printf("未知错误，系统无法识别您的身份");
			return;	
		}
	}
	return;
}

static void rcvBackDataCreate(int fdback)//创建函数接收服务器返回信息判断是否能登录
{
	char dataBack[1];
	bzero(dataBack,sizeof(dataBack));
	
	usleep(100000);

	if(read(fdback,dataBack,1) < 0)
	{
		printf("nothing back!\n");
		return;
	}else
	{
		printf("reading....\n");
		printf("Backmsg: %d\n",atoi(dataBack));
		if(atoi(dataBack) == 1)
		{	
			system("clear");
			printf("创建成功\n");
			printf("%10s\033[1;32m%s\033[0m\n"," ","【欢迎加入会员】");
			return;
		}else if(atoi(dataBack) == 2)
		{
			system("clear");
			printf("创建失败\n");
			printf("%10s\033[1;32m%s\033[0m\n"," ","该账户名已被占用！");
			return;
		}else
		{
			printf("未知错误，系统无法识别您的身份");
			return;	
		}
	}
	return;
}

static void chat_rcv(void *p)	//登陆之后接收服务器发送的聊天信息(线程)
{
	int rcvfd = (int)p;

	struct chat *msg;
	msg = malloc(sizeof(struct chat));

	while(1)
	{
		usleep(100000);
		int cnt = read(rcvfd,(void*)msg,sizeof(struct chat));

		if(cnt <= 0)
		{
//			printf("nothing back!\n");
//			break;
		}else
		{
			printf("\n");
			printf("%s said: %s\n",msg->launch_name,msg->chat_msg);
//			return;
		}
	}	
	free(msg);
	return;
}

void packageSend(void *p,int sockfd)	//单次发送数据包函数
{
	size_t size;
	if((size = write(sockfd,(void*)p, 1024)) < 0)
	{
		perror("Write error!数据包写入错误");
		exit(1);
	}
//	printf("\n数据包发送成功！%d\n",(int)size);
}

void chat_to(int chatfd)		//发送聊天数据包
{
	Protocol *chatMsgSend;
	chatMsgSend = (struct protocol*)calloc(1,sizeof(struct protocol));

	pthread_t id;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&id,&attr,(void *)chat_rcv,(void*)chatfd);

	while(1)
	{
		chatMsgSend->type = 3;
		printf("%10s\033[1;31m%s\033[0m\n"," ","温馨提示:退出请输入exit 按回车键结束!");
		printf("\n请输入您想要聊天的对象:");
		fgets(chatMsgSend->chatName,30,stdin);
		if(strcmp("exit\n",chatMsgSend->chatName) == 0)
		{
			break;
		}
		chatMsgSend->chatName[strlen(chatMsgSend->chatName)-1] = '\0';
		printf("请输入您想要发送的信息:");
		fgets(chatMsgSend->data,1024,stdin);
		if(strcmp("exit\n",chatMsgSend->data) == 0)
		{
			break;
		}
		chatMsgSend->data[strlen(chatMsgSend->data)-1] = '\0';

		packageSend(chatMsgSend,chatfd);
		sleep(1);
	}

	free(chatMsgSend);
	pthread_cancel(id);
}

void sendExit(int exitfd)		//发送退出指令数据包
{
	Protocol *exitSend;
	exitSend = (struct protocol*)calloc(1,sizeof(struct protocol));

	exitSend->type = 0;
	strcpy(exitSend->exit, "exit");

	packageSend(exitSend,sockfd);

	free(exitSend);
}

void heartBeat(int signo)//心跳数据包打包并发送,使用信号捕捉方式
{
	Protocol *heartBeatSend;
	heartBeatSend = (struct protocol*)calloc(1,sizeof(struct protocol));

	heartBeatSend->type = 4;
	
	time_t timenow;
	time(&timenow);

	strcpy(heartBeatSend->heart, ctime(&timenow));

	packageSend(heartBeatSend,sockfd);

	free(heartBeatSend);

	alarm(3);
}

void createNewUser(int sockfd)	//客户端新建用户，将信息传入结构体msgSend
{
	Protocol *userAccount;
//	memset(userAccount, 0, sizeof(Protocol));
	userAccount = (struct protocol*)calloc(1,sizeof(struct protocol));
	int i = 0;
	char pd[128] = {0};
	
	printf("%30s\033[1;32m%s\033[0m\n"," ","=====创建新用户=====");
	userAccount->type = 2;
	printf("请输入新账户名:");
	fgets(userAccount->userName,30,stdin);
	userAccount->userName[strlen(userAccount->userName)-1] = '\0';

	printf("请输入密码:");
	for(i=0;;i++)
	{
		pd[i] = getch();
		if(pd[i] == '\n')
		{
			pd[i] = '\0';
			break;
		}
		if(pd[i] == 127)
		{
			printf("\b \b");
			i = i-2;
		}else
		{
			printf("*");
		}
		if(i < 0)
		{
			pd[0]='\0';
		}
	}
	printf("\n您输入的密码为: %s",pd);
	strcpy(userAccount->userPwd,pd);

	packageSend(userAccount,sockfd);//将结构体数据打包为数据包发送到服务器

	free(userAccount);
}


