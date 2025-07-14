#include "user.pb.h"
#include "friend.pb.h"
#include <mprpcapplication.h>
#include <iostream>
#include <vector>
#include "userinfo.h"
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <zookeeperutil.h>
#include "chatservice.pb.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include"usersqlite.h"
#include"localmessage.pb.h"
#include"group.pb.h"


class RpcClient
{
public:
    RpcClient(std::string filename);
    void OnConnection(const muduo::net::TcpConnectionPtr &conn);
    void OnMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buffer,
                   muduo::Timestamp time);
    bool Login(int userid, std::string password);
    void ConnectChatService(muduo::net::EventLoop &loop);
    std::vector<fixbug::Friends> GetFriendList(int userid);
    std::vector<fixbug::groupInfo> GetGroup(int userid);
    void sendMessage(int method = 1, int friendid = -1, std::string message = "");
    void sendGroupMessage(int method = 2, int groupid = -1,std::string message = "");
    std::string timestampToDatetime(time_t timestamp);
    void createGroup();
    void joingroup(int groupid);
    void GetOfflinemsg(int userid);
    ~RpcClient();

private:

    muduo::net::TcpClient *tcpclient_;
    UserInfo *user_;
    fixbug::UserServiceRpc_Stub *userstub_;
    fixbug::FiendServiceRpc_Stub *friendstub_;
    fixbug::RpcGroupChat_Stub *group_;
    muduo::net::TcpConnectionPtr myconn_;
    SqliteModel *sqlmodel;
};