

#include"public.h"
#include"CommonConnection.h"
#include<string>

//线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool;//lock和unlock
	return &pool;
}



//从配置文件中加载配置项

bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("Mysql.ini", "r");

	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist");
		return false;
	}

	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1)//该行为无效的配置项
		{
			continue;
		}

		//找配置项后面的回车
		int endidx = str.find("\n", idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1); 

		if (key == "ip")
		{
			_ip = value;
		}

		else if (key == "port")
		{
			_port = atoi(value.c_str());
		}

		else if (key == "username")
		{
			_username = value;
		}

		else if (key == "password")
		{
			_password = value;
		}

		else if (key == "dbname")
		{
			_dbname = value;
		}

		else if (key == "initSize")
		{
			_initSize = atoi(value.c_str());
		}

		else if (key == "maxSize")
		{
			_maxSize = atoi(value.c_str());
		}

		else if (key == "maxIdleTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}

		else if (key == "connectionTimeOut")
		{
			_connectionTimeOut = atoi(value.c_str());
		}
	}

	return true;
}

//连接池的构造
ConnectionPool::ConnectionPool()
{
	//加载配置项
	if (!loadConfigFile()) return;


	//创建初始连接量
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//刷新一下开始空闲的起始时间
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//启动一个新线程，作为连接的生产者线程
	//绑定类里的对象方便访问类中的成员方法
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	//启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行空余连接回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();

}

//负责产生新连结
void ConnectionPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex); 
		while (!_connectionQue.empty())
		{
			cv.wait(lock);//队列不空，生产者线程等待
		}

		//连接数量没有达到上限，继续创建新的连接
		if (_connectionCnt < _maxSize)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//刷新一下开始空闲的起始时间
			_connectionQue.push(p);
			_connectionCnt++;
		}


		//通知消费者线程，开始消费
		cv.notify_all();
	}
}



//给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPool::getConnection()
{

	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		//不能用SLEEP，在等待时如果能够消费了应当及时消费
		//超过等待时间后仍然没有被唤醒,且队列为空，打印错误报告
		if (cv_status::timeout==cv.wait_for(lock, chrono::microseconds(_connectionTimeOut)))
		{
			if (_connectionQue.empty())
			{
				LOG("获取空闲连接超时了...获取连接失败");
				return nullptr;
			}
		}
	}
	/*
		 shared_ptr智能指针析构的时候，会把connection资源直接delete掉，相当于调用connection的析构函数
		 connection就被close掉了。这里需要自定义shared_ptr的释放资源方式，把connection归还到queue当中
	*/
	shared_ptr<Connection> sp(_connectionQue.front(),
		[&](Connection* pcon) {
			//这里是在服务器应用线程中调用，会有多个线程归还连接，需要考虑线程安全
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();//刷新一下开始空闲的起始时间
			_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	//提醒生产者线程生产
	cv.notify_all();
	

	return sp;
}


// 扫描超过maxIdleTime时间的空闲连接，进行空余连接回收
void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		//通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		//扫描队列释放空闲连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p;
			}


			else break;//队头的连接没有超过_maxIdleTime，其他连接肯定没有
		}
	}
}