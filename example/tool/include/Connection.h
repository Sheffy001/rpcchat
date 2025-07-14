#pragma once
#include<mysql/mysql.h>
#include<string>
#include<ctime>
#include<muduo/base/Logging.h>
class Connection{
public:
    Connection();
    ~Connection();

    bool connect(std::string ip,
    unsigned short port,
    std::string user,
    std::string password,
    std::string dbname);

    bool update(std::string  sql);
    MYSQL_RES*query(std::string sql);

    void RefreshAliveTime(){_alivetime = clock();}
    clock_t getAlivetime(){return clock()-_alivetime;}
    MYSQL* getConnection(){return _conn;}
private:
    MYSQL *_conn;
    clock_t _alivetime;
};