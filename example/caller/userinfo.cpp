#include "userinfo.h"
UserInfo::UserInfo(int id, std::string name)
{
    userid_ = id;
    name_ = name;
}
UserInfo::~UserInfo()
{
}
void UserInfo::printFriendList()
{
    for (auto a : friendsList_)
    {
        std::cout << "好友id:" << a.userid() << " " << "好友昵称:" << a.friendname() << " " << "在线状态:" << a.online() << std::endl;
    }
}
void UserInfo::setFriendList(std::vector<fixbug::Friends> friendlist)
{
    friendsList_ = friendlist;
    printFriendList();
}
void UserInfo::getGroup(std::vector<fixbug::groupInfo> group_)
{
    group = group_;
    printGroup();
}
void UserInfo::printGroup()
{
    for (auto a : group)
    {
        std::cout << "群id:" << a.groupid() << " " << "管理员:" << a.adminid() << " " << "创建时间:" << a.createtime() << "群人数:" << a.usersnum() << std::endl;
    }
}
void UserInfo::printfFriendmsg(fixbug::ChatMessage c)
{
    std::string msg = c.msg();
    std::cout<<"friendid:"<<c.friendid()<<" "<<"friendname:"<<c.friendname()<<" "<<"msg:"<<msg<<" time:"<<c.send_time()<<std::endl;
}
void UserInfo::printfGroupmsg(fixbug::GroupMessage c)
{
    std::string msg = c.msg();
    std::cout<<"groupid:"<<c.groupid()<<" "<<"memberid:"<<c.memberid()<<" membername:" <<c.membername() <<"msg:"<<msg<<" time:"<<c.send_time()<<std::endl;
}