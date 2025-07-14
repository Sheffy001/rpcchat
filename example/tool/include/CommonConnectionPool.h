#pragma once
#include<string>
#include<queue>
#include"Connection.h"
#include<atomic>
#include<mutex>
#include<thread>
#include<memory>
#include<functional>
#include<condition_variable>
#include<muduo/base/Logging.h>
using namespace std;
class ConnectPool{
public:
    static ConnectPool* getInstance();
    shared_ptr<Connection> getConnection();
private:
    ConnectPool();
    bool LoadConfigFile();
    void produceConnectionTask();
    void scannerConnection();


private:
    string _ip;
    unsigned short _port;
    string _username;
    string _password;
    string _dbname;
    int _maxSize;
    int _initSize;
    int _maxIdleTime;
    int _connectionTimeout;

    atomic_int _connectionCnt;
    mutex _queueMutex;
    queue<Connection*> _connectqueue;
    condition_variable cv;

};