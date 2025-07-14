#include"iostream"
#include"sqlite3.h"
#include"localmessage.pb.h"
#include"vector"
#include<string>
class SqliteModel{
private:
    sqlite3 *sq;
    int rc = -1;
    char * errmsg;
public:
    SqliteModel(std::string file);
    bool createTable(std::string type,int id);
    bool deleteMessage(std::string name,int userid,std::string username,std::string msg, uint64_t timestamp);
    bool deleteTable(std::string name);
    bool insertTable(std::string name,int userid,std::string username,std::string msg, uint64_t timestamp);
    std::vector<fixbug::Msg>  queryTable(std::string name);
    std::vector<fixbug::Msg>  queryTable(std::string name,int userid,std::string username,std::string msg, uint64_t timestamp);
    std::vector<fixbug::Msg>  queryTable(std::string name,std::string msg);
    ~SqliteModel();
};