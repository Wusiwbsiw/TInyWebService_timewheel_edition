#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include"../lock/locker.h"
#include"../CGImysql/sql_connection_pool.h"
#include"../log/block_queue.h"
#include"../log/log.h"
#include<netinet/in.h>



/*

    先制作一个简单的
    每个用户当前只能发送给另外一个已经存在的用户
    然后定时刷新显示，显示数据库中id，id对应的content，显示按照时间排序的最新的10条内容
    用户<---->服务器<---->用户

    存储历史数据的数据库
    comm_his(user_id1 int NULL, user_id2 int NULL,comm_content TEXT NULL,comm_time TIMESTAMP NULL)ENGINE=InnoDB;

*/

struct Message {
    int user_id1,user_id2;         // 双方的id号
    struct sockaddr_in trans_addr; // 通信的端的地址
    struct sockaddr_in recei_addr; // 通信的端的地址
    std::string content;           // 传输内容
    time_t timestamp;              // 时间戳
};


class MessageHandler{
private:
    block_queue<Message> *m_mes_queue; //消息队列
    locker m_mutex;
    bool m_close_mes;
    bool m_mes_buf_size;
    char* m_mes_buf;
    bool m_is_async; 
    int m_close_log = 0;
    

    string m_User;		 //登陆数据库用户名
    string m_PassWord;	 //登陆数据库密码
	string m_DatabaseName; //使用数据库名

private:
    MessageHandler();
    ~MessageHandler();

    bool write_mes();
    bool read_mes();

public:

    void init_mesqueue(int max_mes_size,bool close_mes,int mes_buf_size);
    void init_db(string User, string PassWord, string DBName, int close_log);

    // 双方在线且套接子通信成功
    bool legal_comm();
    // 发送信息
    bool send();
    // 接受信息
    bool receive();



};

#endif