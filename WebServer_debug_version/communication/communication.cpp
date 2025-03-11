#include"communication.h"


string User = "yqy"; 
string PassWord = "Aa001111"; 
string DBName = "comm_his";


void MessageHandler::init_mesqueue(int max_mes_size,bool close_mes,int mes_buf_size){
        m_mes_queue = new block_queue<Message>(max_mes_size);
        m_close_mes = close_mes;
        m_mes_buf_size = mes_buf_size;
        m_mes_buf = new char[m_mes_buf_size];
        memset(m_mes_buf, '\0', m_mes_buf_size);
};

void MessageHandler::init_db(string User, string PassWord, string DBName,  int close_log){

	m_User = User;
	m_PassWord = PassWord;
	m_DatabaseName = DBName;

    MYSQL* con = NULL;
    con = mysql_init(con);
    if (con == NULL)
	{
		LOG_ERROR("MySQL Init Error");	
        exit(1);
	}

    con = mysql_real_connect(con, NULL, User.c_str(), PassWord.c_str(), DBName.c_str(), 0, NULL, 0);
	if (con == NULL)
	{
		LOG_ERROR("MySQL Connect Error");
		exit(1);
	}

}


bool MessageHandler::legal_comm(){
    
}