

#include"public.h"
#include"CommonConnection.h"
#include<string>

//�̰߳�ȫ���������������ӿ�
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool;//lock��unlock
	return &pool;
}



//�������ļ��м���������

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
		if (idx == -1)//����Ϊ��Ч��������
		{
			continue;
		}

		//�����������Ļس�
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

//���ӳصĹ���
ConnectionPool::ConnectionPool()
{
	//����������
	if (!loadConfigFile()) return;


	//������ʼ������
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//ˢ��һ�¿�ʼ���е���ʼʱ��
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//����һ�����̣߳���Ϊ���ӵ��������߳�
	//������Ķ��󷽱�������еĳ�Ա����
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	//����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����п������ӻ���
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();

}

//�������������
void ConnectionPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex); 
		while (!_connectionQue.empty())
		{
			cv.wait(lock);//���в��գ��������̵߳ȴ�
		}

		//��������û�дﵽ���ޣ����������µ�����
		if (_connectionCnt < _maxSize)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//ˢ��һ�¿�ʼ���е���ʼʱ��
			_connectionQue.push(p);
			_connectionCnt++;
		}


		//֪ͨ�������̣߳���ʼ����
		cv.notify_all();
	}
}



//���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
shared_ptr<Connection> ConnectionPool::getConnection()
{

	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		//������SLEEP���ڵȴ�ʱ����ܹ�������Ӧ����ʱ����
		//�����ȴ�ʱ�����Ȼû�б�����,�Ҷ���Ϊ�գ���ӡ���󱨸�
		if (cv_status::timeout==cv.wait_for(lock, chrono::microseconds(_connectionTimeOut)))
		{
			if (_connectionQue.empty())
			{
				LOG("��ȡ�������ӳ�ʱ��...��ȡ����ʧ��");
				return nullptr;
			}
		}
	}
	/*
		 shared_ptr����ָ��������ʱ�򣬻��connection��Դֱ��delete�����൱�ڵ���connection����������
		 connection�ͱ�close���ˡ�������Ҫ�Զ���shared_ptr���ͷ���Դ��ʽ����connection�黹��queue����
	*/
	shared_ptr<Connection> sp(_connectionQue.front(),
		[&](Connection* pcon) {
			//�������ڷ�����Ӧ���߳��е��ã����ж���̹߳黹���ӣ���Ҫ�����̰߳�ȫ
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();//ˢ��һ�¿�ʼ���е���ʼʱ��
			_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	//�����������߳�����
	cv.notify_all();
	

	return sp;
}


// ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����п������ӻ���
void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		//ͨ��sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		//ɨ������ͷſ�������
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


			else break;//��ͷ������û�г���_maxIdleTime���������ӿ϶�û��
		}
	}
}