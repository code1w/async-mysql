
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
#include <cassert>
#include <mysql/mysql.h>
#include "mysql_io_service.h"
#include "mysql_db.h"
#include "mysql_result.h"
#include "mysql_result_row.h"

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

	// 测试网络驱动
    gamesh::mysql::MysqlIOService* loop = new gamesh::mysql::MysqlIOService(1);

	// 测试myslq查询器
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
		
		//query("select * from bag;", mysql);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    //::testing::InitGoogleTest(&argc, argv);
    //return RUN_ALL_TESTS();
    return 0;
}
