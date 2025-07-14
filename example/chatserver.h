#pragma once
#include <string>
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/EventLoop.h>
#include "CommonConnectionPool.h"
#include <queue>
#include <condition_variable>
#include <mutex>
#include "chatservice.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include<any>
#include"zookeeperutil.h"
#include"mysqlmodel.h"
#include"redis.hpp"
#include<atomic>
class ChatService
{
private:
    muduo::net::TcpServer *_tcpserver;
    MysqlModel *mysqlpool_;
    std::unordered_map<int, muduo::net::TcpConnectionPtr > _userMap;
    std::mutex _mmutex;
    std::queue<pair<int, fixbug::ChatMessage>> _sendMsg;
    std::mutex _qmutex;
    muduo::net::EventLoop _loop;
    std::string ip;
    uint32_t port;
    std::condition_variable _qcv;
    Redis _redis;
    std::atomic_bool _stop = true;
    
    

public:
    ChatService(std::string ip, uint32_t port,int n);
    void OnMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buffer,
                   muduo::Timestamp receiveTime);
    void OnConnection(const muduo::net::TcpConnectionPtr &conn);
    void HandleRedisMessage(int userid,const char* msg);

    ~ChatService();
};