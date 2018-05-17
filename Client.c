/*
 * @ author: Shiyipaisizuo
 * @ email: shiyipaisizuo
 * @ lastmodifly: 2017-12-21 10:21
 * @ tool: vim + gcc
 * @ version v0.3.3
 * 介绍:
 *      1.导入头文件Public.h
 *      2.调用安全线程函数创建子线程
 *      3.新增缓冲函数，解决卡死状态
 *      4.优化显示界面
 *      5. 注意:此版本基于UNIX环境，Windows环境下需要修改头文件
 */

#include"Public.h"

pthread_t tid1; // 安全创建子线程

char g_name[NAME_LEN]; // 姓名长度
int  g_locate; // 地址
int  g_total; // 总体

void flush() // 释放缓冲
{
    char c;
    do
    {
        c = getc(stdin);
    }
    while(c != '\n' && c != EOF);
};

int CheckExist() // 检查用户登录状态
{

    int i;

    for(i = 0; i < MAX_CLIENT; i++)
    {

        if(!strcmp(g_name, clientList[i].name))
            break;
    }

    if(i < MAX_CLIENT)
    {
        printf("用户: %s 现已退出!\n", g_name);
        return 1;
    }
    else
        return 0;
}

void  ShowList() // 显示在线用户列表
{
    int i;

    g_total = 0;

    printf("\t _____________________________ \n");
    printf("\t|         连接   列表         |\n");
    printf("\t|_____________________________|\n");
    printf("\t|\t  顺序   |      名字         \t\n ");
    for(i = 0; i < MAX_CLIENT; i++)
    {

        if(clientList[i].socketFd != 0)
        {
            if(i == g_locate)
            {
                printf("\t\t   *%-4d |  %-10s       \t\n", ++g_total, clientList[i].name);
            }
            else
            {
                printf("\t\t   %-4d  |  %-10s       \t\n", ++g_total, clientList[i].name);
            }
        }
    }

    printf("\t\t             共有:%-3d用户    \t\n", g_total);
}

int MakeTempList(int *tmp) // 生成临时连接表
{
    int i,n = 0;
    for(i = 0; i <MAX_CLIENT; i++)
    {
        if(clientList[i].socketFd != 0)
        {
            tmp[n] = i;
            n++;
        }
    }

    ShowList(); // 显示列表

    int select;
    printf("请输入您想聊天的对象(序号) \n");
    if(1!=scanf("%d", &select))
    {
        flush();
        printf("选择出错 \n");
        return -1;
    }
    if(select <= g_total)
    {
        if(tmp[select-1] == g_locate)
        {
            printf("\t系统: 您不能选择自己！\t\n");
            return -1;
        }
        else
            return tmp[select-1];
    }
    else
    {
        printf("选择出错 \n");
        return -1;
    }

}

void *RecvMsg(void *fd)
{

    int sockfd = *(int *)fd; // 网络地址指针

    MESSAGE msg;

    while(1)
    {
        bzero(&msg,sizeof(msg)); // 清空指针

        msg.type = ERROR;

        read(sockfd, &msg, sizeof(msg)); // 读取信息

        if(msg.type == ERROR)
            break;
        switch(msg.type)
        {

        case LOGIN:
            if(msg.fromUserLocate == g_locate)
                printf("\t××××××  > 登录成功！\t\n");
            else
                printf("\t#服务器加入了  -> 来自:%-10s \t\n", msg.fromUser);

            break;
        case EXIT:
            printf("\t#退出   -> 来自:%-10s 消息:%s\t\n", clientList[msg.fromUserLocate].name, msg.message);

            break;
        case PUBLIC:
            printf("\t#公共 -> 来自:%-10s 消息:%s\t\n", msg.fromUser, msg.message);

            break;
        case PRIVATE:
            printf("\t#私人 -> 来自:%-10s 消息:%s\t\n", msg.fromUser, msg.message);

            break;
        default:
            break;

        }
        memcpy(&clientList, &msg.clientList, sizeof(clientList)); // 内存拷贝函数

    }

    printf("Bye! \n");
    exit(1);

}

void SendMsg(int fd)
{

    MESSAGE msg;

    msg.type = LOGIN;
    msg.fromUserLocate = g_locate;

    strcpy(msg.fromUser, g_name);
    strcpy(msg.message, g_name);

    write(fd, &msg, sizeof(msg));

    int tmp[MAX_CLIENT];
    int  key;

    while(1)
    {

        printf(" 1: 公共聊天室 \n 2: 私人聊天室 \n 3: 退出 \n 4: 在线人数\n");

        if(1!= scanf("%d",&key))
        {
            key = 0;

            flush();
        }
        bzero(&msg,sizeof(msg));

        strcpy(msg.fromUser, g_name);

        msg.fromUserLocate = g_locate;

        switch(key)
        {

        case 1:
            msg.type=PUBLIC;

            printf("\n公共聊天室：请发言 \n");

            flush();
            fgets(msg.message,sizeof(msg.message),stdin);

            msg.message[strlen(msg.message)-1]='\0';

            write(fd,&msg,sizeof(msg));
            break;

        case 2:
            bzero(tmp,sizeof(tmp));

            msg.type=PRIVATE;

            if(-1 != (msg.sendUserLocate = MakeTempList(tmp)))
            {
                printf("\n私人聊天室: 请选择你想私聊的对象 \n");

                flush(); // 释放

                fgets(msg.message,sizeof(msg.message),stdin); // 获取信息写入字节流

                msg.message[strlen(msg.message)-1] = '\0';

                write(fd, &msg, sizeof(msg)); // 写入信息
            }
            break;

        case 3:
            printf("退出 \n");

            msg.type=EXIT;

            strcpy(msg.message,"再见");

            write(fd,&msg,sizeof(msg));

            break;

        case 4:
            ShowList();

            break;

        default:
            printf("错误的选择  \n");

            msg.type=0;

            break;
        }

        if(msg.type == EXIT)
        {
            break;
        }
    }

    pthread_cancel(tid1); // 取消子线程


}
int main()
{
    int fd;

    char ip[20] = "127.0.0.1"; // ip地址

    struct sockaddr_in addr;

    addr.sin_port = htons(PORT); // 连接Port
    addr.sin_family = AF_INET; // 联网
    addr.sin_addr.s_addr = inet_addr(ip); // 连接服务器

    if(-1 == (fd = socket(AF_INET, SOCK_STREAM, 0))) // 套接字字节流
    {
        perror("网络出错");
        exit(1);
    }

    if(-1 == (connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr)))) // 检查连接
    {
        perror("连接出错");
        exit(2);
    }

    MESSAGE msg;

    read(fd, &msg, sizeof(msg)); // 读取信息

    if(msg.type == EXIT)
    {
        printf("服务器拒绝连接 \n");
        exit(1);
    }
    else
    {

        memcpy(&clientList, &msg.clientList, sizeof(clientList)); // 检测信息长度与用户状态

        g_locate = msg.fromUserLocate; // 与用户地址名相对应

        pthread_create(&tid1, NULL, RecvMsg, (void *)&fd);
        do
        {
            printf("欢迎进入，请输入您的名字:\n");

            scanf("%s",g_name);
        }
        while(CheckExist()); // 检查状态

        SendMsg(fd);

        pthread_join(tid1, NULL); // 将子进程加入安全线程
    }
    return 0;
}
