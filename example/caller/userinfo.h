#include<iostream>
#include<string>
#include<vector>
#include"../user.pb.h"
#include"../friend.pb.h"
#include<map>
#include<unordered_map>
#include"../group.pb.h"
class UserInfo{
public:
    UserInfo(int id,std::string name);
    ~UserInfo();
    void printFriendList();
    void setFriendList(std::vector<fixbug::Friends> friendlist);
    int getUserid(){return userid_;}
    std::string getUsername(){return name_;}
    void getGroup(std::vector<fixbug::groupInfo> group_);
    void printGroup();

    void printfFriendmsg(fixbug::ChatMessage);
    void printfGroupmsg(fixbug::GroupMessage);
private:
    int userid_ = -1;
    std::string name_;
    std::vector<fixbug::Friends> friendsList_;
    std::unordered_map<int,std::vector<std::string>> messageLog_;
    std::vector<fixbug::groupInfo> group;
    

};