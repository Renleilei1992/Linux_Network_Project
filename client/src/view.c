/*

客户端菜单程序

Author : Renleilei
Date   : 2016-12-10
Version: 1.0

*/

#include "view.h"
#include <stdio.h>


void mainMenu(void)
{
	printf("%30s\033[1;32m%s\033[0m\n"," ","=====欢迎进入LOWBI聊天系统====="," ");
	printf("\n");
	printf("%32s\033[1;32m%s\033[0m\n"," ","|1,用户登录  |");
	printf("%32s\033[1;32m%s\033[0m\n"," ","|2,新用户注册|");
	printf("%32s\033[1;32m%s\033[0m\n"," ","|3,退出系统  |");

	return ;
}

void userMenu(void)
{
	printf("%30s\033[1;32m%s\033[0m\n"," ","=====内测功能=====");
	printf("%32s\033[1;32m%s\033[0m\n"," ","1,在线好友列表");
	printf("%32s\033[1;32m%s\033[0m\n"," ","2,聊天");
	printf("%32s\033[1;32m%s\033[0m\n"," ","3,命令");
	printf("%32s\033[1;32m%s\033[0m\n"," ","4,退出");
}
