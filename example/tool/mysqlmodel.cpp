#include "mysqlmodel.h"

bool MysqlModel::UserExist(int userid, std::string pwd)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where userid = %d and password = '%s'", userid, pwd.c_str());
    MYSQL_RES *res = conn->getConnection()->query(sql);
    if (res == nullptr)
    {
        return false;
    }
    return true;
}
void MysqlModel::updateStatus(int userid, int statue)
{
    char sql[1024] = {0};
    sprintf(sql, "UPDATE user SET online = %d WHERE userid = %d;", statue, userid);
    conn->getConnection()->update(sql);
}
void MysqlModel::insertOffline(int userid, std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offline_message(userid,msg)  values(%d,'%s')",
            userid, msg.c_str());
    conn->getConnection()->update(sql);
}
// void insertUser(fixbug::ChatMessage msg);

std::vector<fixbug::Friends> MysqlModel::getFriends(int userid)
{
    std::vector<fixbug::Friends> friends;
    char sql[1024] = {0};
    sprintf(sql, "select userid,nickname,online from user where userid in (select friend_userid from friendship where userid = %d)",
            userid);
    MYSQL_RES *res = conn->getConnection()->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            fixbug::Friends f;
            f.set_userid(atoi(row[0]));
            f.set_online(row[2]);
            f.set_friendname(row[1]);
            friends.push_back(f);
        }
    }
    return friends;
}
bool MysqlModel::insertUser(int userid, std::string name, std::string pwd)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into user(userid,username,password) values(%d,'%s','%s') ", userid, name.c_str(), pwd.c_str());
    return conn->getConnection()->update(sql);
}
bool MysqlModel::addGroup(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser(groupid,userid) values(%d,%d) ", groupid,userid);
    return conn->getConnection()->update(sql);
}
std::vector<std::string> MysqlModel::getOfflinemsg(int userid){
    std::vector<std::string>msg;
    char sql[1024] = {0};
    sprintf(sql, "select msg from offline_message where userid = %d ", userid);
    MYSQL_RES* res = conn->getConnection()->query(sql);
    if(res!=nullptr){
        MYSQL_ROW row;
        while(row = mysql_fetch_row(res)){
            msg.push_back(row[0]);
        }
    }
    return msg;
}
void MysqlModel::getAllgroup(std::vector<int>&v){
    char sql[1024] = {0};
    sprintf(sql, "select groupid from groupinfo");
    MYSQL_RES* res = conn->getConnection()->query(sql);
    if(res!=nullptr){
        MYSQL_ROW row;
        while(row = mysql_fetch_row(res)){
            v.push_back(atoi(row[0]));
        }
    }
}
std::vector<int> MysqlModel::getMember(int groupid)
{
    std::vector<int> users;
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d ", groupid);
    MYSQL_RES *res = conn->getConnection()->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {

            users.push_back(atoi(row[0]));
        }
    }
    return users;
}
void MysqlModel::getGroup(int userid, std::vector<fixbug::groupInfo> &v)
{

    char sql[1024] = {0};
    sprintf(sql, "select groupid,adminid,createtime,usernum from groupinfo where groupid in(select groupid from groupuser where userid = %d) ", userid);
    MYSQL_RES *res = conn->getConnection()->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            fixbug::groupInfo info;
            info.set_groupid(atoi(row[0]));
            info.set_adminid(atoi(row[1]));
            info.set_createtime(atoi(row[2]));
            info.set_usersnum(atoi(row[3]));
            v.push_back(info);
        }
    }
}
int MysqlModel::insertGroup(fixbug::groupInfo info)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupinfo(adminid,createtime,usernum) values(%d, FROM_UNIXTIME(%ld),1) ", info.adminid(),info.createtime());
    std::shared_ptr<Connection> c = conn->getConnection();
    c->update(sql);
    my_ulonglong id = mysql_insert_id(c->getConnection());
    return id;
}
void MysqlModel::getGroup(std::map<int, std::vector<int>> &mp)
{
    char sql[1024] = {0};
    sprintf(sql, "select groupid,adminid from groupuser");
    MYSQL_RES *res = conn->getConnection()->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            mp[atoi(row[0])].push_back(atoi(row[1]));
        }
    }
}
int MysqlModel::getOnline(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select online from user where userid = %d", userid);
    MYSQL_RES *res = conn->getConnection()->query(sql);
    MYSQL_ROW row;
    if (res != nullptr)
    {

        row = mysql_fetch_row(res);
        return atoi(row[0]);
    }
    return -1;
}

void MysqlModel::addFrient(int userid,int friendid){
    char find[1024] = {0};
    std::shared_ptr<Connection> coo = conn->getConnection();
    sprintf(find, "select * from friendship where userid = %d and friend_userid = %d ", userid,friendid);
    if(coo->query(find)!=nullptr)return;
    char sql[1024] = {0};
    char sql2[1024] = {0};
    sprintf(sql, "insert into friendship(userid,friend_userid) values(%d, %d) ", userid,friendid);
    sprintf(sql2, "insert into friendship(userid,friend_userid) values(%d, %d) ", friendid,userid);
    coo->update(sql);
    coo->update(sql2);
}