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
	���ӳ�ģ��
 */


class ConnectionPool
{
public:
	//��ȡһ�����ӳض���
	static ConnectionPool* getConnectionPool();

	//���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
	shared_ptr<Connection> getConnection();
private:

	//����ģʽ�����캯��˽�л�
	ConnectionPool();

	//�������ļ��м���������
	bool loadConfigFile();

	//�����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();

	//ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����п������ӻ���
	void scannerConnectionTask();

	string _ip;//mysql��ip��ַ
	unsigned short _port;//�˿ں�  3306
	string _username;//mysql�Ķ˿ں�
	string _password;//mysql��¼����
	string _dbname;//���ӵ����ݿ�����
	int _initSize;//��ʼ������
	int _maxSize; //���������
	int _maxIdleTime;//������ʱ��
	int _connectionTimeOut;//��ȡ���ӵĳ�ʱʱ��


	queue<Connection*> _connectionQue;//�洢mysql���ӵĶ���

	mutex _queueMutex;//ά���̰߳�ȫ�Ļ�����

	atomic_int _connectionCnt;//��¼������������connection���������ó���maxSize

	condition_variable cv;//�����������������������������߳����������̵߳�ͨѶ
};


