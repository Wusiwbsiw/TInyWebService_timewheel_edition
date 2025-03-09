#ifndef CONNECTION_POOL_
#define CONNECTION_POOL_

#include<stdio.h>
#include<list>
#include<mysql/mysql.h>
#include<error.h>
#include<string.h>
#include<iostream>
#include<string>
#include"../lock/locker.h"
#include"../log/log.h"

using namespace std;

class connection_pool{
private:
    int m_MaxConn;
    int m_CurConn;
    int m_FreeConn;
    locker lock;
    list<MYSQL*> connList;
    sem reserve;
public:
    string m_url;
    string m_Port;
    string m_User;
    string m_PassWord;
    string m_DatabaseName;
    int m_close_log;
private:
    connection_pool();
    ~connection_pool();
public:
    MYSQL *GetConnection();
    bool ReleaseConnection(MYSQL *conn);
    int GetFreeConn();
    void DestoryPool();

    static connection_pool *GetInstance();

    void init(string url,string User,string PassWord,string DBName,int port,int MaxConn,int close_log);

};

class connectionRAII{
private:
    MYSQL* conRAII;
    connection_pool* poolRAII; 
public:
    connectionRAII(MYSQL **con,connection_pool* connPool);
    ~connectionRAII();
};

#endif