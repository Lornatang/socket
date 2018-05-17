/*
 * @ author: Shiyipaisizuo
 * @ email: shiyipaisizuo@gmail.com
 * @ lastmodifly: 2017-12-20 23:45
 * @ tool: Vim + gcc
 * @ version: v0.2.0
 * 介绍:
 *      1.定义头文件Public.h
 *      2.导入标准头文件
 *      3.新增C语言中socket网络以及类型模块
 *      4.调用系统封装的I/O处理模块(非stdio.h)
 *      5.导入多线程模块
 *      6.最大连接数为3
 *      7.姓名最大长度为20
 *      8.消息最大长度为100
 *      9.Port地址为12345
 *      10. 注意:此版本基于Unix环境，Windows环境下需要修改头文件
 */

#ifndef PUBLIC_H_ //如果未定义，则定义，定义则重新定义
#define PUBLIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/socket.h"
#include "sys/types.h"
#include "arpa/inet.h"
#include "netinet/in.h"
#include <unistd.h>
#include <pthread.h>


#define  MAX_CLIENT 8
#define  NAME_LEN   30
#define  MSG_LEN    1024
#define  PORT       8888

#define LOGIN   1
#define EXIT    2
#define PUBLIC  3
#define PRIVATE 4
#define OK      5


#define ADD     7
#define DEL     8
#define ERROR   -1
#define FIFO    "FIFO"


typedef struct ClientList
{
    char name[NAME_LEN]; // 姓名长度
    int socketFd; // 端口
} CLIENTLIST; // 定义CLIENTLIST结构体用来存储连接用户的信息

typedef struct Message
{
    char fromUser[NAME_LEN]; // 发送用户的名字
    int  fromUserLocate; // 发送用户的地址
    int  type; // 类型
    int  sendUserLocate; // 发送用户的地址
    char message[MSG_LEN]; // 消息长度
    CLIENTLIST clientList[MAX_CLIENT]; // 检测连接数是否已满
} MESSAGE; // 定义用户消息结构体


CLIENTLIST  clientList[MAX_CLIENT]; // 检测功能


#endif
