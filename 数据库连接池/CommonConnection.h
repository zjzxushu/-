#pragma once
#include<string>
#include<queue>
#include"Connection.h" 
#include<mutex>
#include<atomic>
#include<thread>
#include<memory>
#include<functional>
#include<condition_variable>
using namespace std;



/*
	连接池模块
 */


class ConnectionPool
{
public:
	//获取一个连接池对象
	static ConnectionPool* getConnectionPool();

	//给外部提供接口，从连接池中获取一个可用的空闲连接
	shared_ptr<Connection> getConnection();
private:

	//单例模式，构造函数私有化
	ConnectionPool();

	//从配置文件中加载配置项
	bool loadConfigFile();

	//运行在独立的线程中，专门负责生成新连接
	void produceConnectionTask();

	//扫描超过maxIdleTime时间的空闲连接，进行空余连接回收
	void scannerConnectionTask();

	string _ip;//mysql的ip地址
	unsigned short _port;//端口号  3306
	string _username;//mysql的端口号
	string _password;//mysql登录密码
	string _dbname;//连接的数据库名称
	int _initSize;//初始连接量
	int _maxSize; //最大连接量
	int _maxIdleTime;//最大空闲时间
	int _connectionTimeOut;//获取连接的超时时间


	queue<Connection*> _connectionQue;//存储mysql连接的队列

	mutex _queueMutex;//维护线程安全的互斥锁

	atomic_int _connectionCnt;//记录连接所创建的connection总数，不得超过maxSize

	condition_variable cv;//设置条件变量，用于连接生产者线程与消费者线程的通讯
};


