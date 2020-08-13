#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdio>
#include <signal.h>
#include <mysql/mysql.h>
#include <functional>
#include <thread>
#include <mutex>

#ifdef _WIN32
#include <Winsock2.h>
typedef SOCKET fd_t;
#else
#include <sys/select.h>
typedef int fd_t;
#endif /* _WIN32 */

typedef struct mysql_context_s {
	MYSQL* m ;
	int qindex = 0;
	int rindex = 0;
	char name[32];

} mysql_context_t;

std::vector<mysql_context_t*> conns;
std::vector<std::string> query_sql;
std::mutex mutex;

void query(const std::string& sql,MYSQL *m);
void mysql_write(MYSQL* m);
void mysql_read_result(MYSQL*);
int hand_times = 0;

// ========================================================
// select 

fd_set m_rd_set;
fd_set m_wr_set;
std::unordered_map<fd_t, mysql_context_t*> m_tracked_sockets;
std::vector<fd_t> m_polled_fds;
int init_poll_fds_info(void);
void process_events(void);
void select_rd_event_cb(fd_t, mysql_context_t);
void select_wr_event_cb(fd_t, mysql_context_t);

void query_in_loop(const std::string& sql)
{
	std::lock_guard<std::mutex> l(mutex);
	query_sql.push_back(sql);
}

int init_poll_fds_info(void) {

	m_polled_fds.clear();
	FD_ZERO(&m_rd_set);
	FD_ZERO(&m_wr_set);

	int ndfs = 0;
	for (const auto& socket : m_tracked_sockets) {
		const auto& fd          = socket.first;
		const auto& socket_info = socket.second;

		FD_SET(fd, &m_rd_set);
		//FD_SET(fd, &m_wr_set);
		m_polled_fds.push_back(fd);
		if(fd > ndfs)
		{
			ndfs = (int)fd;
		}
	}
	return ndfs + 1;
}


void mysql_write(MYSQL* m)
{
	query("select * from bag;", m);
}

void mysql_read_result(MYSQL* m) {
	while (true)
	{
		if (mysql_read_query_result(m) == 0) {
			MYSQL_RES *res = mysql_store_result(m);
			MYSQL_ROW row;
			int fields = mysql_num_fields(res);
			while ((row = mysql_fetch_row(res)))
			{
				for (int i = 0; i < fields; i++)
				{
					printf("%s \t", row[i]);
				}
				printf("\n");
			}
			//printf("a result for m->net.fd = %d \n", m->net.fd);
			mysql_free_result(res);
		} else {
			std::cout <<" mysql_read_query_result() failed for fd = "<< m->net.fd << " "<<mysql_error(m)<< ":"<<mysql_errno(m)<< std::endl;
			return;
		}

		switch(mysql_next_result(m))
		{
		case -1:
			return;
		case 0:
			continue;
		default:
			return;
		}

	}
}


void connect_mysql_server(mysql_context_t* context)
{
	context->m = mysql_init(nullptr);
	if (!context->m)
	{
		std::cout<<"mysql init error"<< std::endl;
		return;
	}
	context->m->free_me = 1;
	auto ret = mysql_real_connect(context->m,"192.168.150.79", "root","123456","gamesh_zxb",3306,NULL,0);
	if (!ret)
	{
		std::cout<<"Mysql conn err : "<<mysql_error(context->m) << ", errno :"<< mysql_errno(context->m)<< std::endl;
		return;
	}
	m_tracked_sockets[context->m->net.fd] = context;
	conns.push_back(context);
}



void query(const std::string& sql,MYSQL *m)
{
	auto ret = mysql_send_query(m, sql.c_str(), sql.size());
	printf("libevent  mysql_send_query  %d\n", m->net.fd);
}

void query(const std::string& sql, mysql_context_t* c)
{
	c->qindex++;
	auto ret = mysql_send_query(c->m, sql.c_str(), sql.size());
	printf("mysql_send_query ret : %d \n", ret);
}


void select_rd_event_cb(fd_t fd, mysql_context_t* c)
{
	mysql_read_result(c->m);
	c->rindex++;
	//printf("Result Contesx index %d\n", c->rindex);
}

void select_wr_event_cb(fd_t fd, mysql_context_t* c)
{

}


void hand_select_event(void) {
	static int index = 0;
	index++;
	printf("hand_select_event index %d\n", index);

	for (const auto& fd : m_polled_fds) {

		auto it = m_tracked_sockets.find(fd);

		if (it == m_tracked_sockets.end()) { continue; }

		auto& socket = it->second;

		if (FD_ISSET(fd, &m_rd_set)) {
			select_rd_event_cb(fd, socket);
		}
		if (FD_ISSET(fd, &m_wr_set)) {
			select_wr_event_cb(fd, socket);
		}
	}
}

void process_query()
{
	if (query_sql.empty())
	{
		return;
	}

	std::lock_guard<std::mutex> l(mutex);
	std::string sql = query_sql.back();
	query_sql.pop_back();
	query(sql, conns[0]);
}


void poll()
{
	while (true) {
		int ndfs = init_poll_fds_info();
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1000; //1ms
		if (select(ndfs, &m_rd_set, &m_wr_set, NULL, &tv) > 0) {
			hand_select_event();
		}
		else {
			process_query();
		}
	}
}

void test_select()
{
	std::thread worker = std::thread(poll);

	for (int i = 0; i < 5; i++)
	{
		mysql_context_t* c = new mysql_context_t;
		connect_mysql_server(c);
	}

	for (int i = 0; i < 100; i++)
	{
		query_in_loop(std::string("select * from account;"));
		//query("select * from account", conns[0]);
	}
	worker.join();
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	if (WSAStartup(version, &data) != 0) {
		std::cerr << "WSAStartup() failure" << std::endl;
		return -1;
	}
#endif

	//test_libevent();
	//test_libuv();
	mysql_library_init(0, nullptr, nullptr);
	test_select();
	mysql_server_end();
	return 0;
}