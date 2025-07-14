
#include"CommonConnectionPool.h"

ConnectPool* ConnectPool::getInstance(){
    static ConnectPool p;
    return &p;
}

ConnectPool::ConnectPool(){
    if(!(LoadConfigFile())){
        return;
    }
    for(int i = 0;i<_initSize;i++){
        Connection *p = new Connection();
        p->connect(_ip,_port,_username,_password,_dbname);
        p->RefreshAliveTime();
        _connectqueue.push(p);
        _connectionCnt++;
    }

    //启动一个生产者线程
    thread produce(std::bind(&ConnectPool::produceConnectionTask,this));
    produce.detach();
    //启动一个监督线程
    thread scannerConnecton_(std::bind(&ConnectPool::scannerConnection,this));
    scannerConnecton_.detach();

}

bool ConnectPool::LoadConfigFile(){
    FILE *pf= fopen("/home/sheffy/CppProject/RPCCHAT/example/callee/mysql.ini","r");
    if(pf==nullptr){
        //LOG("mysql.ini file is not exit!");
        LOG_ERROR<<"mysql.ini file is not exit!";
        return false;
    }
    while(!feof(pf)){
        char line[1024]={0};
        fgets(line,1024,pf);
        string str = line;

        int idex = str.find('=');
        int endidx = str.find('\n');
        if(idex==string::npos)continue;

        string key = str.substr(0,idex);
        string value = str.substr(idex+1,endidx-idex-1);

        if(key=="username"){
            _username = value;
        }
        else if(key=="password"){
            _password = value;
        }
        else if(key=="ip"){
            _ip=value;
        }
        else if(key=="port"){
            _port = atoi(value.c_str());
        }
        else if(key=="dbname"){
            _dbname=value;
        }
        else if(key=="maxSize"){
            _maxSize = atoi(value.c_str());
        }
        else if(key=="initSize"){
            _initSize = atoi(value.c_str());
        }
        else if(key=="maxIdleTime"){
            _maxIdleTime = atoi(value.c_str());
        }
        else if(key=="connectionTimeout"){
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

void ConnectPool::produceConnectionTask(){
    while(1){
        unique_lock<mutex>lock(_queueMutex);
        while(!_connectqueue.empty()){
            cv.wait(lock);
        }
        if(_connectionCnt<_maxSize){
            Connection *p = new Connection();
            p->connect(_ip,_port,_username,_password,_dbname);
            p->RefreshAliveTime();
            _connectqueue.push(p);
            _connectionCnt++;
        }
        cv.notify_all();
    }
}

shared_ptr<Connection> ConnectPool::getConnection(){
    unique_lock<mutex>lock(_queueMutex);
    while(_connectqueue.empty()){
        if(cv_status::timeout ==cv.wait_for(lock,chrono::milliseconds(_connectionTimeout)))
        {
            if(_connectqueue.empty()){
                LOG_ERROR<<"获取连接超时！";
            //LOG();
            return nullptr;
            }
        }
    }
    shared_ptr<Connection> sp(_connectqueue.front(),[&](Connection*pcon){
        unique_lock<mutex>lock(_queueMutex);
        pcon->RefreshAliveTime();
        _connectqueue.push(pcon);
    });
    _connectqueue.pop();
    cv.notify_all();
    return sp;
}

void ConnectPool::scannerConnection(){ 
    while(1){
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));
        unique_lock<mutex>lock(_queueMutex);
        while(_connectionCnt>_initSize){
            Connection*p = _connectqueue.front();
            if(p->getAlivetime()>=_maxIdleTime*1000){
                _connectqueue.pop();
                _connectionCnt--;
                delete p;
            }
            else{
                break;
            }
        }
    }
}




