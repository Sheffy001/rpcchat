#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"
#include "mysqlmodel.h"
class FriendService : public fixbug::FiendServiceRpc
{
public:
    std::vector<fixbug::Friends> GetFriendsList(uint32_t userid)
    {
        return mysqlpool.getFriends(userid);
    }

    // 重写基类方法
    void GetFriendsList(::google::protobuf::RpcController *controller,
                        const ::fixbug::GetFriendsListRequest *request,
                        ::fixbug::GetFriendsListResponse *response,
                        ::google::protobuf::Closure *done)
    {
        uint32_t userid = request->userid();
        std::vector<fixbug::Friends> friendsList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (auto &name : friendsList)
        {
            fixbug::Friends *p = response->add_friends();
            p->set_friendname(name.friendname());
            p->set_online(name.online());
            p->set_userid(name.userid());
        }
        done->Run();
    }
    void Addfriend(::google::protobuf::RpcController *controller,
                        const ::fixbug::AddfriendRequest *request,
                        ::fixbug::AddfriendResponse *response,
                        ::google::protobuf::Closure *done)
    {
        mysqlpool.addFrient(request->userid(),request->friendid());
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("添加成功");
        done->Run();
    }


private:
    MysqlModel mysqlpool;
};

int main(int argc, char **argv)
{

    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run("rpcfriendip", "rpcfriendport");

    return 0;
}