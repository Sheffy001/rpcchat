#include "../group.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include"../redis.hpp"
#include"../mysqlmodel.h"

class GroupChat : public fixbug::RpcGroupChat
{
public:
    GroupChat(){
        redis_ = new Redis();
        redis_->connect();
        initGroup();
    }
    bool Add(int userid,int groupid){
        return model.addGroup(userid,groupid);

    }
    void Sendmsg(fixbug::GroupMessage msg){
        
        int groupid = msg.groupid();
        auto it = group.find(groupid);
        if(it!= group.end()){
            for(auto a:group[groupid]){
                fixbug::GroupMessage msg1 = msg;
                msg1.set_userid(a);
                std::string sendmsg;
                msg1.SerializeToString(&sendmsg);
                uint32_t msgsize = sendmsg.size();
                std::string send_msg;
                send_msg.append((char*)&msgsize,4);
                send_msg+=sendmsg;
                redis_->publish(a,send_msg);
            }
        }
    }
    void getGroup(int userid,std::vector<fixbug::groupInfo>&v){
        model.getGroup(userid,v);
    }

    void createGroup(::google::protobuf::RpcController *controller,
                  const ::fixbug::createGroupRequest *request,
                  ::fixbug::createGroupResponse *response,
                  ::google::protobuf::Closure *done)
    {
        fixbug::groupInfo info;
        info.set_adminid(request->userid());
        info.set_createtime(request->createtime());
        int groupid = model.insertGroup(info);
        Add(request->userid(),groupid);
        response->set_groupid(groupid);
        done->Run();
    }
    void GetGroup(::google::protobuf::RpcController *controller,
                  const ::fixbug::getGroupRequest *request,
                  ::fixbug::getGroupResponse *response,
                  ::google::protobuf::Closure *done)
    {
        int userid = request->userid();
        std::vector<fixbug::groupInfo> v;
        getGroup(userid,v);
        for (auto &name : v)
        {
            fixbug::groupInfo *p = response->add_group();
            *p = name;
        }
        done->Run();
    }
    void AddGroup(::google::protobuf::RpcController *controller,
                  const ::fixbug::AddGroupRequest *request,
                  ::fixbug::AddGroupResponse *response,
                  ::google::protobuf::Closure *done)
    {
        int userid = request->userid();
        int groupid = request->groupid();
        if(Add(userid,groupid)){
            fixbug::ResultCode* res = response->mutable_result();
            res->set_errcode(0);
            res->set_errmsg("加入成功");
        }
        else{
            fixbug::ResultCode* res = response->mutable_result();
            res->set_errcode(1);
            res->set_errmsg("加入失败");
        }
        done->Run();
    }
    void SendMsg(::google::protobuf::RpcController *controller,
                 const ::fixbug::SendMessageRequest *request,
                 ::fixbug::SendMessageResponse *response,
                 ::google::protobuf::Closure *done)
    {
        
        Sendmsg(request->msg());
        done->Run();

    }
    void initGroup(){
        std::vector<int>groupid;
        std::vector<int>menberid;
        model.getAllgroup(groupid);
        for(auto a:groupid){
            std::vector<int>menberid = model.getMember(a);
            for(auto b:menberid){
                group[a].push_back(b);
            }
        }
    }

private:
    Redis *redis_;
    std::unordered_map<int,std::vector<int>> group;
    MysqlModel model;
    
 };

 int main(int argc,char** argv){
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new GroupChat());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run("groupserviceip", "groupserviceport");

    return 0;
 }