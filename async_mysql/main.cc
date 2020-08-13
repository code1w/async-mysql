#include <gtest/gtest.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <minwindef.h>
#endif 

#include <iostream>
#include <string>
#include <functional>
#include <cstdio>
#include <chrono>
#include <thread>
#include <mysql/mysql.h>
#include "gamesh_mysql_ioc/mysql_io_service.h"
#include "gamesh_mysql_ioc/mysql_db.h"
#include "gamesh_mysql_ioc/mysql_result.h"
#include "gamesh_mysql_ioc/mysql_result_row.h"


void mysql_read_result(MYSQL* m) {
	if (0 == mysql_read_query_result(m)) {
		MYSQL_RES *res = mysql_store_result(m);
		MYSQL_ROW row;
		int fields = mysql_num_fields(res);
		while ((row = mysql_fetch_row(res)))
		{
			for (int i = 0; i < fields; i++)
			{
				printf("%s\t", row[i]);
			}
		}
		std::thread::id tid = std::this_thread::get_id();
		printf("a result for m->net.fd = %d  tid = %d \n", m->net.fd, tid);
		mysql_free_result(res);
	} else {
		std::cout <<" mysql_read_query_result() failed for fd = "<< m->net.fd << " "<<mysql_error(m)<< ":"<<mysql_errno(m)<< std::endl;
	}
}

void query(const std::string& sql,MYSQL *m)
{
	std::thread::id tid = std::this_thread::get_id();
	auto ret = mysql_send_query(m, sql.c_str(), sql.size());
	printf("query  mysql_send_query fd =  %d  tid =  %d \n", m->net.fd, tid);
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    WORD version = MAKEWORD(2, 2);
    WSADATA data;

    if (WSAStartup(version, &data) != 0)
    {
        std::cerr << "WSAStartup() failure" << std::endl;
        return -1;
    }
#endif

	std::string myslq_host = "";
#ifdef _WIN32
	myslq_host = "192.168.150.79";
#else 
	myslq_host = "172.17.0.13";
#endif 

	// ²âÊÔÍøÂçÇý¶¯
    gamesh::mysql::MysqlIOService* loop = new gamesh::mysql::MysqlIOService(1);
	MYSQL* mysql = mysql_init(nullptr);
	MYSQL* ret = mysql_real_connect(mysql, myslq_host.c_str(), "root","123456","gamesh_zxb",3306,nullptr,0);

	auto readcb = [](void* ud) {
		MYSQL* mysql = (MYSQL*)ud;
		mysql_read_result(mysql);
	};
	
	//loop->Track(mysql->net.fd, mysql, readcb, nullptr);


	// ²âÊÔmyslq²éÑ¯Æ÷
	gamesh::mysql::MysqlDB* db = new gamesh::mysql::MysqlDB(loop);
	db->syncConnect(myslq_host, "root","123456","gamesh_zxb",3306);
	
	db->onConnected([](const char *error) {
		if (error) std::cout << "Failed to connect: " << error << std::endl;
		else std::cout << "Connected" << std::endl;
	});

#if 1
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif 
    while (true)
    {
		db->query("select * from account");
		/*
		db->query("select * from account").onSuccess([](gamesh::mysql::Result&& result) {
			assert(result.affectedRows() == 0);
			for (auto row : result)
			{
				std::cout << "{" << std::endl;
				for (auto field : row)
				{
					std::cout << "  " << field.first << " => " << field.second << std::endl;
				}
				std::cout << "}" << std::endl;
			}
			}).onFailure([](const char *error) {
				std::cout << "Query error: " << error << std::endl;
		});
		*/
		//query("select * from bag;", mysql);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    //::testing::InitGoogleTest(&argc, argv);
    //return RUN_ALL_TESTS();
    return 0;
}
