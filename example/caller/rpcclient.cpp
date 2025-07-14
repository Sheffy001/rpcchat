
#include "rpcclient.hpp"

RpcClient::RpcClient(std::string filename)
{
    sqlmodel = new SqliteModel(filename);
    userstub_ = new fixbug::UserServiceRpc_Stub(new MprpcChannel());
    friendstub_ = new fixbug::FiendServiceRpc_Stub(new MprpcChannel());
    group_ = new fixbug::RpcGroupChat_Stub(new MprpcChannel());
    ZkClient::getInstance()->Start();
}
void RpcClient::ConnectChatService(muduo::net::EventLoop &loop)
{
    std::string host_data = ZkClient::getInstance()->GetData("/CHAT/CHAT");
    if (host_data == "")
    {

        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {

        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    muduo::net::InetAddress address_(ip, port);

    tcpclient_ = new muduo::net::TcpClient(&loop, address_, "client");
    tcpclient_->setConnectionCallback(std::bind(&RpcClient::OnConnection, this, std::placeholders::_1));
    tcpclient_->setMessageCallback(std::bind(&RpcClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    tcpclient_->connect();
    tcpclient_->enableRetry();
}
void RpcClient::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        std::cout << "Connected to Chat Server!" << std::endl;

        // 向服务端发送上线通知（比如封装一个 ChatMessage）
        fixbug::ChatMessage msg;
        msg.set_method(0); // method 0 表示登录通知
        msg.set_userid(user_->getUserid());

        std::string body;
        msg.SerializeToString(&body);

        uint32_t len = body.size();
        std::string send_data;
        send_data.append((char *)&len, 4); // 4字节长度头
        send_data += body;
        myconn_ = conn;
        myconn_->send(send_data);
    }
    else
    {
        std::cout << "Disconnected from Chat Server" << std::endl;
    }
}
void RpcClient::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                          muduo::net::Buffer *buffer,
                          muduo::Timestamp time)
{
    std::cout << "有消息来了" << std::endl;
    while (buffer->readableBytes() >= 4)
    {
        uint32_t net_header_size = 0;
        memcpy(&net_header_size, buffer->peek(), sizeof(net_header_size));

        if (buffer->readableBytes() < 4 + net_header_size)
            break;

        buffer->retrieve(4);
        std::string body = buffer->retrieveAsString(net_header_size);

        fixbug::ChatMessage msg;
        
        msg.ParseFromString(body);
        if(msg.method()==1){
            user_->printfFriendmsg(msg);
            sqlmodel->insertTable("friend" + std::to_string(msg.friendid()), msg.friendid(), msg.friendname(), msg.msg(), msg.send_time());
        }
        else if(msg.method()==2){
            fixbug::GroupMessage gmsg;
            gmsg.ParseFromString(body);
            user_->printfGroupmsg(gmsg);
            sqlmodel->insertTable("group" + std::to_string(gmsg.groupid()), gmsg.memberid(), gmsg.membername(), gmsg.msg(), gmsg.send_time());
        }

       
    }
}
void RpcClient::createGroup()
{
    fixbug::createGroupRequest request;
    request.set_userid(user_->getUserid());
    int64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    request.set_createtime(timestamp);
    // rpc方法的响应
    fixbug::createGroupResponse response;
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    group_->createGroup(nullptr, &request, &response, nullptr);

    std::cout << "rpc createGroup response success:" << std::endl;

    std::vector<fixbug::groupInfo> groups = GetGroup(user_->getUserid());
    user_->getGroup(groups);
}

void RpcClient::joingroup(int groupid)
{
    fixbug::AddGroupRequest request;
    request.set_userid(user_->getUserid());
    request.set_groupid(groupid);
    fixbug::AddGroupResponse response;
    google::protobuf::Closure *done;
    group_->AddGroup(nullptr, &request, &response, done);
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success:" << std::endl;
        std::vector<fixbug::groupInfo> groups = GetGroup(user_->getUserid());
        user_->getGroup(groups);
    }
}
std::string RpcClient::timestampToDatetime(time_t timestamp)
{
    std::tm *tm_time = std::localtime(&timestamp);
    std::ostringstream oss;
    oss << std::put_time(tm_time, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
bool RpcClient::Login(int userid, std::string password)
{
    fixbug::LoginRequest request;
    request.set_username(userid);
    request.set_pwd(password);
    // rpc方法的响应
    fixbug::LoginResponse response;
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    userstub_->Login(nullptr, &request, &response, nullptr);
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success:" << response.sucess() << std::endl;
        user_ = new UserInfo(userid, response.name());
        std::vector<fixbug::Friends> friends = GetFriendList(userid);
        user_->setFriendList(friends);
        std::vector<fixbug::groupInfo> groups = GetGroup(userid);
        user_->getGroup(groups);
        GetOfflinemsg(userid);
        return true;
    }
    else
    {
        // std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
        return false;
    }
}
void RpcClient::GetOfflinemsg(int userid)
{
    fixbug::offlineMsgRequest request;
    request.set_userid(userid);
    // rpc方法的响应
    fixbug::offlineMsgResponse response;
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    userstub_->Getoffline(nullptr, &request, &response, nullptr);

    std::cout << "rpc Getoffline response success:" << std::endl;
    for (int i = 0; i < response.msg_size(); i++)
    {
        std::string msg = response.msg(i);
        if (msg.size() > 4)
        {
            msg = msg.substr(4);
            fixbug::ChatMessage cmessage;
            
            cmessage.ParsePartialFromString(msg);
            if (cmessage.method()==1)
            {
                user_->printfFriendmsg(cmessage);
                sqlmodel->insertTable("friend" + std::to_string(cmessage.friendid()), cmessage.friendid(), cmessage.friendname(), cmessage.msg(), cmessage.send_time());
            }
            else if (cmessage.method()==2)
            {
                fixbug::GroupMessage gmessage;
                gmessage.ParseFromString(msg);
                user_->printfGroupmsg(gmessage);
                sqlmodel->insertTable("group" + std::to_string(gmessage.groupid()), gmessage.memberid(), gmessage.membername(), gmessage.msg(), gmessage.send_time());
            }
        }
    }
}

void RpcClient::sendMessage(int method, int friendid, std::string message)
{
    if (!myconn_ || !myconn_->connected())
    {
        std::cerr << "连接尚未建立，无法发送消息！" << std::endl;
        return;
    }
    fixbug::ChatMessage msg;
    msg.set_method(method);
    msg.set_userid(friendid);
    msg.set_friendid(user_->getUserid());
    msg.set_friendname(user_->getUsername());
    msg.set_msg(message);
    int64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    msg.set_send_time(timestamp);

    std::string buffer;
    msg.SerializeToString(&buffer);
    uint32_t len = buffer.size();
    std::string send_data;
    send_data.append((char *)&len, 4);
    send_data += buffer;
    myconn_->send(send_data);
    sqlmodel->insertTable("friend" + std::to_string(friendid), msg.friendid(), msg.friendname(), msg.msg(), msg.send_time());
}

void RpcClient::sendGroupMessage(int method, int groupid, std::string message)
{
    fixbug::SendMessageRequest request;
    fixbug::GroupMessage *res = request.mutable_msg();
    res->set_memberid(user_->getUserid());
    res->set_method(2);
    res->set_groupid(groupid);
    res->set_membername(user_->getUsername());
    res->set_msg(message);
    int64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    res->set_send_time(timestamp);
    fixbug::SendMessageResponse response;
    google::protobuf::Closure *done;
    group_->SendMsg(nullptr, &request, &response, done);
}

std::vector<fixbug::groupInfo> RpcClient::GetGroup(int userid)
{
    std::vector<fixbug::groupInfo> friendlist;
    fixbug::getGroupRequest request;
    request.set_userid(userid);
    // rpc方法的响应
    fixbug::getGroupResponse response;
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    MprpcController controller;
    group_->GetGroup(&controller, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读调用的结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {

        std::cout << "rpc GetFriendsList response success!" << std::endl;
        int size = response.group_size();
        for (int i = 0; i < size; ++i)
        {
            friendlist.push_back(response.group(i));
            std::cout << response.group(i).groupid() << std::endl;
            sqlmodel->createTable("group", response.group(i).groupid());
        }
    }
    return friendlist;
}
std::vector<fixbug::Friends> RpcClient::GetFriendList(int userid)
{
    std::vector<fixbug::Friends> friendlist;
    fixbug::GetFriendsListRequest request;
    request.set_userid(userid);
    // rpc方法的响应
    fixbug::GetFriendsListResponse response;
    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    MprpcController controller;
    friendstub_->GetFriendsList(&controller, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读调用的结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc GetFriendsList response success!" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; ++i)
            {
                friendlist.push_back(response.friends(i));
                std::cout << response.friends(i).userid() << std::endl;
                sqlmodel->createTable("friend", response.friends(i).userid());
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response error : " << response.result().errmsg() << std::endl;
        }
    }
    return friendlist;
}
RpcClient::~RpcClient()
{
    delete userstub_;
    delete friendstub_;
    delete tcpclient_;
    delete sqlmodel;
}
