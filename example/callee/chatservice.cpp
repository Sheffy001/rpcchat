#include"iostream"
#include"../chatserver.h"


int main(int argc,char** argv){
     // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("chatserviceip");
    uint32_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("chatserviceport").c_str());
    ChatService(ip,port,5);
    return 0;
}