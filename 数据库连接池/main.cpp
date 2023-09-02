#include"Connection.h"
#include<iostream>
#include"CommonConnection.h"

using namespace std;

int main()
{
	

//不使用连接池
#if 0
	clock_t begin = clock();
	for (int i = 0; i < 1000; ++i)
	{
		
		Connection conn;
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");

		conn.connect("172.20.22.124", 3306, "root", "123456", "chat");
		conn.update(sql);
		
	}

#endif

//使用连接池
#if 0
	clock_t begin = clock();
	ConnectionPool* cp = ConnectionPool::getConnectionPool();
	for (int i = 0; i < 1000; ++i)
	{
		shared_ptr<Connection> sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");

		sp->update(sql);

	}

	clock_t end = clock();

	cout << (end - begin) << "ms" << endl;
	
#endif

//多线程使用连接池
#if 0
	clock_t begin = clock();
	


	thread t1([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 250; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);

		}
		});
	thread t2([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 250; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);

		}
		});
	thread t3([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 250; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);

		}
		});
	thread t4([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 250; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);

		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	

	clock_t end = clock();

	cout << (end - begin) << "ms" << endl;

#endif

//多线程不使用线程池
#if 1


	Connection conn;
	conn.connect("172.20.22.124", 3306, "root", "123456", "chat");

	clock_t begin = clock();
	thread t1([]() {
		
		for (int i = 0; i < 250; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");

			conn.connect("172.20.22.124", 3306, "root", "123456", "chat");
			conn.update(sql);

		}
		});
	thread t2([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 250; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");

			conn.connect("172.20.22.124", 3306, "root", "123456", "chat");
			conn.update(sql);

		}
		});
	thread t3([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 250; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");

			conn.connect("172.20.22.124", 3306, "root", "123456", "chat");
			conn.update(sql);
		}
		});
	thread t4([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 250; ++i)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhangsan", 20, "M");

			conn.connect("172.20.22.124", 3306, "root", "123456", "chat");
			conn.update(sql);

		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();


	clock_t end = clock();

	cout << (end - begin) << "ms" << endl;

#endif
	return 0;
}