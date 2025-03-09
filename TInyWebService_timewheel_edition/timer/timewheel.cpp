#include"timewheel.h"



long long TimeWheel::getCurMs(TimePos_t timePos){
    return timePos.pos_ms*1000/m_msec_levelcount 
    + timePos.pos_sec*60*1000/m_seco_levelcount
    + timePos.pos_min*60*60*1000/m_minu_levelcount
    + timePos.pos_hour*60*60*24*1000/m_hour_levelcount;
}

int TimeWheel::createId(){
    return m_id++;
}

void TimeWheel::getTimePos(long long interval,TimePos_t& timePos){
    long long curTime = getCurMs(m_timePos);
    int futureTime = curTime + interval;
    timePos.pos_ms = (futureTime%1000)*m_msec_levelcount/1000;
    timePos.pos_sec = (futureTime/1000%60)*m_seco_levelcount/60;
    timePos.pos_min = (futureTime/1000/60%60)*m_minu_levelcount/60;
    timePos.pos_hour = (futureTime/1000/60/60%24)*m_hour_levelcount*24;
    return;
}

void TimeWheel::insert(long long interval,Event_t& event){
    printf("TimeWheel::insert() begin\n");
    TimePos_t timePos = {0};

    getTimePos(interval,timePos);

    //printf("timePos.pos_hour=%d, m_timePos.pos_hour=%d\n", timePos.pos_hour, m_timePos.pos_hour);
    //printf("timePos.pos_min=%d, m_timePos.pos_min=%d\n", timePos.pos_min, m_timePos.pos_min);
    //printf("timePos.pos_sec=%d, m_timePos.pos_sec=%d\n", timePos.pos_sec, m_timePos.pos_sec);
    //printf("timePos.pos_ms=%d, m_timePos.pos_ms=%d\n", timePos.pos_ms, m_timePos.pos_ms);

    if (timePos.pos_hour != m_timePos.pos_hour){
        printf("index = %d\n",m_msec_levelcount+m_seco_levelcount+m_minu_levelcount+timePos.pos_hour);
        m_eventSlotList[m_msec_levelcount+m_seco_levelcount+m_minu_levelcount+timePos.pos_hour].push_back(event);
    }else if (timePos.pos_min != m_timePos.pos_min){
        printf("index = %d\n",m_msec_levelcount+m_seco_levelcount+timePos.pos_min);
        m_eventSlotList[m_msec_levelcount+m_seco_levelcount+timePos.pos_min].push_back(event);
    }else if (timePos.pos_sec != m_timePos.pos_sec){
        printf("index = %d\n",m_msec_levelcount+timePos.pos_sec);
        m_eventSlotList[m_msec_levelcount+timePos.pos_sec].push_back(event);
    }else if (timePos.pos_ms != m_timePos.pos_ms){
        printf("index = %d\n",timePos.pos_ms);
        m_eventSlotList[timePos.pos_ms].push_back(event);
    }

    printf("TimeWheel::insert() end\n");

    return;
}

int TimeWheel::process(std::list<Event_t>& eventList){
    //printf("int TimeWheel::process(std::list<Event_t>& eventList)\n");
    for (auto event = eventList.begin();event != eventList.end();event ++){
        long long curMs = getCurMs(m_timePos);
        long long preMs = getCurMs(event->timePos);
        long long difMs = std::max(curMs - preMs,0ll);
        printf("curMs = %d,preMs = %d,difMs = %d\n",curMs,preMs,difMs);

        if (event->interval >= difMs){
            printf("in if\n");
            event->cb();
            printf("callback fun runed\n");
            event->timePos = m_timePos;
            //printf("int TimeWheel::process(std::list<Event_t>& eventList) end\n");
        }else{
            printf("else if\n");
            insert(difMs,*event);
            //printf("int TimeWheel::process(std::list<Event_t>& eventList) end\n");
        }
    }
    
    return 0;
}

TimeWheel::TimeWheel():m_msec_levelcount(100),m_seco_levelcount(60)
                      ,m_minu_levelcount(60),m_hour_levelcount(24)
                      ,m_id(0),m_steps(10){
    memset(&m_timePos,0,sizeof(m_timePos));
}

void TimeWheel::initTimeWheel(int msec_levelcount = 100,int seco_levelcount = 60,int minu_levelcount = 60,int hour_levelcount = 24){
    if (msec_levelcount < 0 || msec_levelcount < 0 || msec_levelcount < 0 || msec_levelcount < 0 ){
        printf("invaild parameters\n");
        return;
    }
    m_eventSlotList.resize(msec_levelcount + seco_levelcount + minu_levelcount + hour_levelcount);
    printf("m_eventSlotList.size = %d\n",m_eventSlotList.size());
    int ret = pthread_create(&m_loopThread,NULL,loopForInterval,this);
    if (ret != 0){
        printf("create thread error:%s\n", strerror(errno));
        return;
    }
    m_msec_levelcount = msec_levelcount;
    m_seco_levelcount = seco_levelcount;
    m_minu_levelcount = minu_levelcount;
    m_hour_levelcount = hour_levelcount;
    m_steps = 1000/msec_levelcount;
    return;
}
 
void TimeWheel::createTimingEvent(long long interval){
    //printf("void TimeWheel::createTimingEvent(long long interval)\n");
    if (interval >= 1000*60*60*24)
    {
        printf("invalid interval\n");
        return;
    }
    Event_t event = {0};
    event.interval = interval;
    //event.cb = callback;
    // set time start
    event.timePos.pos_min = m_timePos.pos_min;
    event.timePos.pos_sec = m_timePos.pos_sec;
    event.timePos.pos_ms = m_timePos.pos_ms;
    event.id = createId();
    m_locker.lock();
    insert(interval,event);
    m_locker.unlock();
}

void *TimeWheel::loopForInterval(void *arg)
{
    if (arg == NULL)
    {
        printf("valid parameter\n");
        return NULL;
    }

    TimeWheel *timeWheel = reinterpret_cast<TimeWheel *>(arg);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeWheel->m_steps));
        TimePos pos = {0};
        TimePos m_lastTimePos = timeWheel->m_timePos;
        // update slot of current TimeWheel
        timeWheel->getTimePos(timeWheel->m_steps, pos);
        timeWheel->m_timePos = pos;
        // if minute changed, process in integral point (minute)
        if (pos.pos_hour != m_lastTimePos.pos_hour)
        {
            //printf("in hour wheel\n");
            timeWheel->m_locker.lock();
            std::list<Event_t> *eventList = &timeWheel->m_eventSlotList[timeWheel->m_timePos.pos_hour + timeWheel->m_minu_levelcount + timeWheel->m_seco_levelcount + timeWheel->m_msec_levelcount];
            timeWheel->process(*eventList);
            eventList->clear();
            timeWheel->m_locker.unlock();
        }else if (pos.pos_min != m_lastTimePos.pos_min){
            //printf("in min wheel\n");
            timeWheel->m_locker.lock();
            std::list<Event_t> *eventList = &timeWheel->m_eventSlotList[timeWheel->m_timePos.pos_min + timeWheel->m_seco_levelcount + timeWheel->m_msec_levelcount];
            timeWheel->process(*eventList);
            eventList->clear();
            timeWheel->m_locker.unlock();
        }else if (pos.pos_sec != m_lastTimePos.pos_sec){
            //printf("in sec wheel\n");
            timeWheel->m_locker.lock();
            std::list<Event_t> *eventList = &timeWheel->m_eventSlotList[timeWheel->m_timePos.pos_sec + timeWheel->m_msec_levelcount];
            timeWheel->process(*eventList);
            eventList->clear();
            timeWheel->m_locker.unlock();
        }else if (pos.pos_ms != m_lastTimePos.pos_ms){
            //printf("in ms wheel\n");
            timeWheel->m_locker.lock();
            std::list<Event_t> *eventList = &timeWheel->m_eventSlotList[timeWheel->m_timePos.pos_ms];
            timeWheel->process(*eventList);
            eventList->clear();
            timeWheel->m_locker.unlock();
        }
        
    }
    return nullptr;
}



void Utils::init(int timeslot){
    m_TIMESLOT = timeslot;
}

int Utils::setnonblocking(int fd){
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void Utils::sig_handler(int sig)
{
    // 为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void Utils::addsig(int sig,void(hanlder)(int),bool restart){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = hanlder;
    if (restart){
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL) != -1);
}

void Utils::timer_handler(){
    alarm(m_TIMESLOT);
}

void Utils::show_error(int  connfd,const char* info){
    send(connfd,info,strlen(info),0);
    close(connfd);
}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;


