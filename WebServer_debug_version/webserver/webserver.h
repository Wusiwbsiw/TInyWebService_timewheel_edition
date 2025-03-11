#ifndef WEBSERVER_H
#define WEBSERVER_H

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<cassert>
#include<sys/epoll.h>

#include"../threadpool/threadpool.h"
#include"../http/http_conn.h"
#include"../timer/timewheel.h"


const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;


class WebServer{
    
public:
    // 基础
    int m_port;
    char* m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    int m_pipefd[2];
    int m_epollfd;
    http_conn* users;
    
    // 数据库相关
    connection_pool* m_connPool;
    string m_user;
    string m_passwd;
    string m_dbname;
    int m_sql_num;

    // 线程池相关
    threadpool<http_conn>* m_pool;
    int m_thread_num;

    // epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

    //定时器相关
    Event_t* users_timer;
    Utils utils;



public:
    WebServer();
    ~WebServer();

    void init(int port,string user,string passwd,string dbname
             ,int log_write,int opt_linger,int trigmode,int sql_num
             ,int thread_num,int close_log,int actor_model);

    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd,struct sockaddr_in client_address);
    void adjust_timer();
    void deal_timer(Event_t *event, int sockfd);
    bool dealclinetdata();
    bool dealwithsignal(bool &timeout, bool &stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

};



#endif