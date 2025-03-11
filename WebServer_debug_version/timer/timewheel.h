#ifndef TIMEWHEEL_H
#define TIMEWHEEL_H


#include<memory>
#include<vector>
#include<list>
#include<iostream>
#include<chrono>
#include<thread>
#include<algorithm>
#include<memory.h>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<signal.h>
#include<assert.h>
#include"../lock/locker.h"
#include"../http/http_conn.h"

typedef struct TimePos{
    int pos_ms;
    int pos_sec;
    int pos_min;
    int pos_hour;
}TimePos_t;


class Event_t;
class TimeWheel{
    typedef std::shared_ptr<TimeWheel> TimeWheelPtr;
    typedef std::vector<std::list<Event_t>> EventSlotList_t;
private:

    EventSlotList_t m_eventSlotList;
    TimePos_t m_timePos;
    pthread_t m_loopThread;

    int m_msec_levelcount;
    int m_seco_levelcount;
    int m_minu_levelcount;
    int m_hour_levelcount;

    int m_id;
    int m_steps;
    locker m_locker;
private:
    long long getCurMs(TimePos_t timePos);
    int createId();
    int process(std::list<Event_t>& eventList);
    void getTimePos(long long interval,TimePos_t& timePos);
    void insert(long long interval,Event_t& event);

public:
    TimeWheel();

    void initTimeWheel(int msec_levelcount,int seco_levelcount,int minu_levelcount,int hour_levelcount);
    void createTimingEvent(long long interval);

    static void* loopForInterval(void* arg);

};

class Utils{

public:
    static int* u_pipefd;
    TimeWheel timewheel;
    static int u_epollfd;
    int m_TIMESLOT;

public:

    Utils(){}
    ~Utils(){}

    void init(int timeslot);

    int setnonblocking(int fd);

    void addfd(int epollfd,int fd,bool one_shot,int TRIGMode);

    static void sig_handler(int sig);

    void addsig(int sig, void(handler)(int), bool restart = true);

    void timer_handler();

    void show_error(int connfd,const char* info);
};

class Event_t{
public:
    int id;
    sockaddr_in address;
    int sockfd;
    void cb(void){
        // 关闭网页时启用
        printf("now in callback fun begin\n");
        epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, sockfd, 0);
        close(sockfd);
        http_conn::m_user_count--;
        printf("now in callback fun end\n");
    };
    void* arg;
    //long long delay;
    TimePos_t timePos;
    int interval;
};


#endif