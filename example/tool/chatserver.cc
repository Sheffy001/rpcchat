#include "chatserver.h"

ChatService::ChatService(std::string ip, uint32_t port, int n)
{
    mysqlpool_ = new MysqlModel();
    muduo::net::InetAddress address(ip, port);
    _tcpserver = new muduo::net::TcpServer(&_loop, address, "chatserver");
    _tcpserver->setConnectionCallback(std::bind(&ChatService::OnConnection, this, std::placeholders::_1));
    _tcpserver->setMessageCallback(std::bind(&ChatService::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _tcpserver->setThreadNum(4);
    ZkClient::getInstance()->Start();
    ZkClient::getInstance()->Create("/CHAT", nullptr, 0);
    ZkClient::getInstance()->Create("/CHAT/CHAT", "127.0.0.1:8002", strlen("127.0.0.1:8002"), 0);
    _redis.connect();
    _redis.init_notify_handler(std::bind(&ChatService::HandleRedisMessage,this,std::placeholders::_1,std::placeholders::_2));
    for (int i = 0; i < n; i++)
    {

        std::thread q([this]()
                      {
                        while(_stop){
                            std::pair<int,fixbug::ChatMessage> task;
                        {
                            std::unique_lock<std::mutex>lock(_qmutex);
                             _qcv.wait(lock,[this]()->bool{return !_sendMsg.empty();});
                            task = _sendMsg.front();
                            _sendMsg.pop();
                        }
                        std::string sendmsg;
                        task.second.SerializeToString(&sendmsg);
                        uint32_t len = sendmsg.size();
                       
                       
                        std::string send_data;
                        send_data.append((char *)&len, 4);
                        send_data += sendmsg;
                        std::cout<<"len"<<len<<std::endl;
                        std::cout<<"send_data:"<<send_data.size()<<std::endl;
                        if(mysqlpool_->getOnline(task.first)==1){
                            _redis.publish(task.first,send_data);
                        }
                        else{
                            mysqlpool_->insertOffline(task.first,send_data);
                        }
        } });
        q.detach();
    }
    _tcpserver->start();
    _loop.loop();
}

void ChatService::HandleRedisMessage(int userid, const char* msg)
{
    std::lock_guard<std::mutex> lock(_mmutex);
    uint32_t len;
    std::memcpy(&len,msg,4);
    std::string buffer(msg,4+len);
    std::cout<<"buffer:"<<buffer.size()<<std::endl;
    auto it = _userMap.find(userid);
    if (it != _userMap.end()) {
        it->second->send(buffer);
    }
}
void ChatService::OnMessage(
    const muduo::net::TcpConnectionPtr &conn,
    muduo::net::Buffer *buffer,
    muduo::Timestamp receiveTime)
{
    while (buffer->readableBytes() >= 4)
    {
        const char *data = buffer->peek();
        uint32_t header_size = 0;
        memcpy(&header_size, data, 4);
        if (buffer->readableBytes() < 4 + header_size)
            break;
        std::string buffersize = buffer->retrieveAsString(4);
        std::string body = buffer->retrieveAsString(header_size);
        std::cout<<"header_size"<<header_size<<std::endl;
        fixbug::ChatMessage msg;
        msg.ParseFromString(body);
        if (msg.method() == 0)
        {
            {
                std::lock_guard<std::mutex> lock(_mmutex);
                _userMap[msg.userid()] = conn;
            }
            _redis.subscribe(msg.userid());

            mysqlpool_->updateStatus(msg.userid(), 1);
        }
        else if (msg.method() == 1)
        {
            std::lock_guard<std::mutex> lock(_qmutex);
            _sendMsg.push({msg.userid(), msg});
            _qcv.notify_one();
        }

    }
}
void ChatService::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        std::lock_guard<std::mutex> lock(_mmutex);
        for (auto a : _userMap)
        {
            if (a.second == conn)
            {
                mysqlpool_->updateStatus(a.first, 0);
                _userMap.erase(a.first);
                _redis.unsubscribe(a.first);
                break;
            }
        }
        conn->shutdown();
    }
}
ChatService::~ChatService()
{
    if (_tcpserver != nullptr)
    {
        _stop = false;
        _qcv.notify_all();
        delete _tcpserver;
        _tcpserver = nullptr;
    }
}