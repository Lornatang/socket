/*
 * @ author: Shiyipaisizuo
 * @ email: shiyipaisizuo
 * @ lastmodifly: 2017-12-21 08:19
 * @ tool: vim + gcc
 * @ version v0.1.2
 * 介绍:
 *      1.导入头文件Public.h
 *      2.新增C语言中线程状态模块
 *      3.添加文件管理安全模块，保证程序稳定性
 *      4. 注意:此版本基于UNIX环境，Windows环境下需要修改头文件
 */

#include "Public.h"
#include "sys/stat.h"
#include <fcntl.h>

struct ARG
{
    int locate; // 地址
    int fifoFd; // 缓冲
}; // 为缓冲区地址封装

int SearchLocate() // 查询地址函数
{

    int i = 0;

    for(i = 0; i < MAX_CLIENT; i++)
    {
        if(clientList[i].socketFd==0) // 检查用户ip地址
            break; // 检查完毕退出循环
    }
    if(i<MAX_CLIENT)
        return i; // 正确则通过
    else
        return ERROR; // 错误则终止运行程序
}

void TransmitMsg(int cmd, int locate, MESSAGE msg) // 用户发送消息的函数
{

    memcpy(&msg.clientList,&clientList,sizeof(clientList)); // 安全检查信息长度是否超过1024

    if(cmd == PRIVATE) // 检查命令行状态
    {

        write(clientList[msg.sendUserLocate].socketFd,&msg,sizeof(msg)); // 检查用户输入长度是否大于1024字节
        printf("\t#私人聊天室->  来自:%-5s  向:  %-5s  消息:%s\t\n",clientList[locate].name,clientList[msg.sendUserLocate].name,msg.message); // 显示私人聊天室的用户发送的消息
    }
    else
    {
        int i;

        for (i = 0; i < MAX_CLIENT; i++) // 检查连接数
        {
            if(clientList[i].socketFd!=0 && i!=locate) // 检查是否有相同地址运行在同一台服务器上
            {
                write(clientList[i].socketFd,&msg,sizeof(msg)); // 检查用户输入长度是否大于1024字节
                printf("\t#公共聊天室  ->  来自:%-5s  向:  %-5s  消息:%s\t\n",clientList[locate].name,clientList[i].name,msg.message); // 显示公共聊天室的用户发送的消息
            }
        }
        if(cmd == LOGIN) // 检查命令行状态
        {
            write(clientList[locate].socketFd,&msg,sizeof(msg)); // 检查消息的长度是否小于1024字节
        }
    }
}


void UpdateList(int cmd , char *name,int locate) // 更新用户信息
{

    if(cmd == ADD) // 检查命令行状态
    {
        strcpy(clientList[locate].name,name); // 复制姓名至原姓名处
        printf("\t增加用户-> 姓名:%-5s  \t\n",clientList[locate].name); // 显示新增用户信息至终端
    }
    else if(cmd == DEL) // 检查命令行状态
    {

        printf("\t*断开连接 -> 名字:%-5s  \t\n",clientList[locate].name); // 显示下线用户至终端
        clientList[locate].socketFd=0; // 调整此ip地址状态
        bzero(clientList[locate].name,NAME_LEN); // 清空指针变量
    }

}

void *RecvMsg(void *arg_t) // 回执信息函数
{
    struct ARG arg=*(struct ARG *)arg_t;

    MESSAGE msg;

    while(1)
    {

        int flag;
        bzero(&msg,sizeof(msg)); // 清空消息指针变量
        msg.type = ERROR;

        read(clientList[arg.locate].socketFd,&msg, sizeof(msg)); // 读取消息

        msg.fromUserLocate = arg.locate; // 获取消息地址码

        if(msg.type == EXIT||msg.type == ERROR)
        {
            if(msg.type==ERROR)
            {
                strcpy(msg.message,"错误！");
                printf("\t*连接:%s 出故障\t\n", clientList[msg.fromUserLocate].name);
                msg.type = EXIT; // 线程退出
            }
            if((flag = write(arg.fifoFd,&msg,sizeof(msg))) == -1)
            {
                perror("写入FIFO文件出错"); // 队列记录连接信息
                exit(1);
            }
            break;
        }

        if((flag=write(arg.fifoFd,&msg,sizeof(msg))) == -1)
        {
            perror("写入FIFO文件出错"); // 队列记录连接信息
            exit(1);
        }
    }

    return NULL;

}
void *SendMsg(void *fd)
{

    int fifoFd;

    if((fifoFd = open(FIFO, O_RDONLY)) == -1)
    {
        perror("写入文件出错"); // 队列文件记录连接信息
        exit(1);
    }

    int flag;

    MESSAGE msg;

    while(1)
    {
        if((flag=read(fifoFd, &msg,sizeof(msg))) == -1)
        {
            perror("读取文件出错");
            exit(2);
        }

        int exit_fd;
        switch(msg.type) // 获取消息类型
        {

        case LOGIN:
            UpdateList(ADD, msg.fromUser, msg.fromUserLocate); // 更新用户状态
            TransmitMsg(LOGIN, msg.fromUserLocate, msg); // 调用发送消息函数
            break;

        case PUBLIC:
            TransmitMsg(PUBLIC, msg.fromUserLocate, msg); // 调用发送消息函数
            break;

        case PRIVATE:
            TransmitMsg(PRIVATE, msg.fromUserLocate, msg); // 调用发送消息函数
            break;

        case EXIT:
            exit_fd = clientList[msg.fromUserLocate].socketFd; // 退出状态设置为2
            UpdateList(DEL, msg.fromUser, msg.fromUserLocate); // 更新用户状态
            TransmitMsg(EXIT, msg.fromUserLocate, msg); // 调用发送消息函数
            close(exit_fd); // 关闭退出线程
            break;

        default:
            printf("数据出错！ %d  \n", msg.type);
            break;
        }
    }

    return NULL;

}


int main()
{
    printf("\n\t服务器已启动.....\n");

    pthread_t tid1, tid2; // 安全分配线程

    int fd, clientfd, wr_fifo;

    socklen_t  sock_len;
    sock_len = sizeof(struct sockaddr_in); // 检查网络服务器状态

    mkfifo(FIFO, O_CREAT|O_EXCL); // 创建队列信息

    pthread_create(&tid1, NULL, SendMsg, NULL); // 创建安全信息子线程

    struct  sockaddr_in server, client;

    server.sin_port = htons(PORT); // Port
    server.sin_family = AF_INET; // Host
    server.sin_addr.s_addr = INADDR_ANY; // address

    if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        perror("网络服务器出错！");
        exit(1);
    }

    if(-1 == bind(fd,(struct sockaddr*)&server, sock_len))
    {
        perror("套接字与端口连接出错！");
        exit(2);
    }

    if(-1 == (listen(fd, MAX_CLIENT+1)))
    {
        perror("监听状态出错");
        exit(3);
    }

    if(-1==(wr_fifo=open(FIFO, O_WRONLY)))
    {
        perror("打开文件出错");
        exit(1);
    }

    while(1)
    {

        if(-1 == (clientfd = (accept(fd, (struct sockaddr*)&client, &sock_len))))
        {
            perror("连接出错！");
            exit(4);
        }

        int locate = -1;

        MESSAGE msg;

        if(-1 == (locate = SearchLocate()))
        {

            printf("\t*请求已经收到但是服务器拒绝连接.\t\n");
            msg.type = EXIT;

            write(clientfd, &msg, sizeof(msg));

        }
        else
        {
            struct ARG arg;

            arg.fifoFd = wr_fifo;
            arg.locate = locate;

            msg.type = OK;
            memcpy(&msg.clientList, &clientList, sizeof(clientList));
            msg.fromUserLocate =locate;

            write(clientfd, &msg, sizeof(msg));

            clientList[locate].socketFd = clientfd;

            pthread_create(&tid1, NULL, RecvMsg, (void *)&arg);

        }
    }

    pthread_join(tid1, NULL); // 加入安全子线程
    pthread_join(tid2, NULL); // 加入安全子线程


    unlink("FIFO"); // 无法链接FIFO文件

    return 0;
}
