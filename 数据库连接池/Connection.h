#pragma once


#include <mysql.h>
#include <string>
using namespace std;
#include "public.h"
#include<ctime>
// ���ݿ������
class Connection
{
public:
	// ��ʼ�����ݿ�����
	Connection();
	
	// �ͷ����ݿ�������Դ
	~Connection();
	
	// �������ݿ�
	bool connect(string ip, 
			unsigned short port,
			string user, 
			string password,
			string dbname);

	// ���²��� insert��delete��update
	bool update(string sql);
	
	// ��ѯ���� select
	MYSQL_RES* query(string sql);

	//ˢ�µ�ǰ�������ӵĴ��ʱ��
	void refreshAliveTime() { _allivetime = clock(); }
	//���ص�ǰ���������Ѿ�����˵�ʱ��
	clock_t getAliveTime() const{ return clock() - _allivetime; }
private:
	MYSQL* _conn; // ��ʾ��MySQL Server��һ������
	clock_t _allivetime;//��¼�������״̬���ӵĳ�ʼ���ʱ��
};