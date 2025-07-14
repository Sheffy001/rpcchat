#include"CommonConnectionPool.h"
#include"chatservice.pb.h"
#include"friend.pb.h"
#include"group.pb.h"
class MysqlModel{
public:
    bool UserExist(int userid,std::string pwd);
    void updateStatus(int userid,int statue);
    void insertOffline(int userid,std::string msg);
    bool insertUser(int userid,std::string name,std::string pwd);
    std::vector<fixbug::Friends>getFriends(int userid);
    bool addGroup(int userid,int groupid);
    int insertGroup(fixbug::groupInfo info);
    std::vector<int> getMember(int group);
    void getGroup(std::map<int,std::vector<int>>&mp);
    void getGroup(int userid,std::vector<fixbug::groupInfo>&v);
    int getOnline(int userid);
    std::vector<std::string> getOfflinemsg(int userid);
    void addFrient(int userid,int friendid);
    void getAllgroup(std::vector<int>&v);
private:
    ConnectPool* conn = ConnectPool::getInstance();
};