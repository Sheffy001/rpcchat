#include "rpcclient.hpp"
#include <mprpcapplication.h>
#include <iostream>
#include <thread>
#include "signal.h"
void EXIT(int sig)
{
    std::cout << "\n收到退出信号 (" << sig << ")，正在清理资源..." << std::endl;

    exit(0); // 正常退出程序
}
void send_chatmsg(RpcClient &client);
void send_groupmsg(RpcClient &client);
void addGroup(RpcClient &client);
int main(int argc, char **argv)
{

    MprpcApplication::Init(argc, argv);
    RpcClient client("friend1.db");

    signal(2, EXIT);
    signal(15, EXIT);
    while (true)
    {
        int userid;
        std::cin >> userid;
        std::string pwd;
        std::cin >> pwd;
        if (!client.Login(userid, pwd))
        {
            std::cout << "登录失败" << std::endl;
            continue;
        }
        else
            break;
    }
    std::cout << "登录成功" << std::endl;

    std::thread loop_([&client]
                      {
        muduo::net::EventLoop lloop;
        client.ConnectChatService(lloop);
        lloop.loop(); });
    loop_.detach();
    while (true)
    {
        int choice;
        std::cin>>choice;
        std::cin.ignore(); // 清除换行符
        switch (choice)
        {
        case 1://向好友发送消息
            send_chatmsg(client);
            break;
        case 2://向群发送消息
            send_groupmsg(client);
            break;
        case 3://创建群
            client.createGroup();
            break;
        case 4://加入群
            addGroup(client);
            break;
        
        default:
            break;
        }
        
    }
    return 0;
}

void addGroup(RpcClient &client){
    int groupid;
    std::cout << "输入想加入群的ID: ";
    std::cin >> groupid;
    std::cin.ignore(); // 清除换行符
    client.joingroup(groupid);
    std::cout<<"加入成功"<<std::endl;
}
void send_chatmsg(RpcClient &client){
    int friendid;
    std::cout << "输入好友ID: ";
    std::cin >> friendid;
    std::cin.ignore(); // 清除换行符

    std::string message;
    std::cout << "输入消息内容: ";
    std::getline(std::cin, message); // 读取整行消息（支持空格）
    client.sendMessage(1, friendid, message);
}
void send_groupmsg(RpcClient &client){
    int groupid;
    std::cout << "输入群ID: ";
    std::cin >> groupid;
    std::cin.ignore(); // 清除换行符

    std::string message;
    std::cout << "输入消息内容: ";
    std::getline(std::cin, message); // 读取整行消息（支持空格）
    client.sendGroupMessage(1, groupid, message);
}
