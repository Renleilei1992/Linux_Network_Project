/*

用户信息链表存储程序

Author : Renleilei
Date   : 2016-12-10
Version: 1.0

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "saveList.h"



//读取链表信息
void readList(userInfo_t *head)
{
	FILE *listFileOpen;
	
	if(NULL == head)
	{
		printf("链表初始化失败!\n");
		return;
	}
	listFileOpen = fopen("src/listFile.db","r");
	
	if(listFileOpen == 0)
	{
		perror("fopen error!");
		exit(1);
	}else
	{
//		printf("用户信息链表打开成功! %d\n",(int)listFileOpen);
		printf("用户信息链表打开成功! \n");
	}


	while(1)
	{
		userInfo_t *newNode = (userInfo_t *)malloc(sizeof(userInfo_t));
		memset(newNode,0,sizeof(userInfo_t));
		int retval = fread(newNode,sizeof(userInfo_t),1,listFileOpen);
		
//		printf("retval = %d\n",retval);	
		if(retval < 1)
		{
			free(newNode);
			fclose(listFileOpen);
			return;
		}
	
/*		printf("readList 初始化测试!\n");
		printf("user_name: %s\n",newNode->user_name);
		printf("user_pwd: %s\n",newNode->user_pwd);*/
		addTail(head,newNode);
	}
}



//创建用户信息存储链表
userInfo_t* createList(int size)
{
	//头节点存储在堆中,定义并初始化
	userInfo_t *head = (userInfo_t *)calloc(1,sizeof(userInfo_t));
	head->next = NULL;	//单向不循环链表,next指向下一个节点
	return head;
}

//创建动态链表存储在线用户
userOL_t* create_ol_list(int size)
{
	userOL_t *head = (userOL_t *)calloc(1,sizeof(userOL_t));
	head->next = NULL;
	return head;
}



//使用链表存储用户账户数据
void userInfoListSave(userInfo_t *head)
{
	FILE *listSave;

	listSave = fopen("src/listFile.db","w");
	printf("文件打开成功!准备写入......\n");
	userInfo_t *last = head->next;
//	userInfo_t *last = head;

	while(last != NULL)
	{
		fwrite(last,sizeof(userInfo_t),1,listSave);
		last = last->next;
	}
	fclose(listSave);
}

//动态链表存储尾插法
void addTail_ol(userOL_t *head,userOL_t *newNode)
{
	userOL_t *last = head;
	while(last->next != NULL)
	{
		last = last->next;
	}
	last->next = newNode;
	newNode->next = NULL;
}

//静态链表存储尾插法
void addTail(userInfo_t *head,userInfo_t *newNode)
{
	userInfo_t *last = head;
	while(last->next != NULL)
	{
		last = last->next;
	}
	last->next = newNode;
	newNode->next = NULL;
}

/*
//销毁链表
void destroyList(userInfo_t **head)
{
	userInfo_t *toFree = NULL;
	userInfo_t *tmp = NULL;
	toFree = (*head)->next;
	while(toFree != NULL)
	{
		tmp = toFree->next;
		free(toFree);
		toFree = tmp;
	}
	free(*head);	//销毁头结点
	*head = NULL;	//将头结点的指针置空，防止出现野指针
}
*/

