#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdio>
#include <signal.h>
#include <mysql/mysql.h>
#include <event.h>
#include <uv.h>
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

#if 0
typedef struct mysql_context_s {
	uv_poll_t poll_handle;
	MYSQL* m ;
	int index = 0;
	char name[32];

} mysql_context_t;

std::vector<mysql_context_t*> conns;
std::unordered_map<unsigned long long , event*> events;
std::vector<std::string> query_sql;
std::mutex mutex;

void query(const std::string& sql,MYSQL *m);
void hand_libevent_event(intptr_t fd, short what, void* _userdata);
void hand_libuv_event(uv_poll_t* handle, int status, int what);
void mysql_write(MYSQL* m);
void mysql_read_result(MYSQL*);
int hand_times = 0;




void hand_libevent_event(intptr_t fd, short what, void* _userdata)
{
	if (what & EV_READ)
	{
		event* ev = events[fd];
		if (ev)
		{
			ev->ev_events |= EV_READ;
			ev->ev_events |= EV_WRITE;
			int ret = event_add(ev, NULL);
		}
		mysql_read_result((MYSQL*)_userdata);
	}
	if (what & EV_WRITE)
	{
		mysql_write((MYSQL*)_userdata);
	}

}

void hand_libuv_event(uv_poll_t* handle, int status, int what)
{
	hand_times++;
	mysql_context_t* context = (mysql_context_t*)handle->data;

	if (what & UV_READABLE)
	{
		printf("hand_libuv_event times %d\n", hand_times);
		mysql_read_result(context->m);
	}
	if (what & UV_WRITABLE)
	{
		mysql_write(context->m);
	}

}

void mysql_write(MYSQL* m)
{
	query("select * from bag;", m);
}

void mysql_read_result(MYSQL* m) {
	//mysql_read_query_result(m);

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
		printf("a result for m->net.fd = %d \n", m->net.fd);
		mysql_free_result(res);
	} else {
		std::cout <<" mysql_read_query_result() failed for fd = "<< m->net.fd << " "<<mysql_error(m)<< ":"<<mysql_errno(m)<< std::endl;
	}
}

void connect_mysql_server()
{
	for (int i = 0; i < 4; i++)
	{
		mysql_context_t* c = new mysql_context_t;

		c->m = mysql_init(nullptr);
		if (!c->m)
		{
			std::cout<<"mysql init error"<< std::endl;
			return;
		}
		c->m->free_me = 1;
		auto ret = mysql_real_connect(c->m,"192.168.150.79", "root","123456","gamesh_zxb",3306,NULL,0);
		if (!ret)
		{
			std::cout<<"Mysql conn err : "<<mysql_error(c->m) << ", errno :"<< mysql_errno(c->m)<< std::endl;
			return;
		}
		conns.push_back(c);
		m_tracked_sockets[c->m->net.fd] = c;
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

void bind_event(struct event_base* evbase)
{
	for (auto c : conns)
	{
		auto e = event_new(evbase, c->m->net.fd, EV_READ | EV_PERSIST, hand_libevent_event, c->m);
		event_add(e, NULL);
		events[c->m->net.fd] = e;
	}

}

void query(const std::string& sql,MYSQL *m)
{
	auto ret = mysql_send_query(m, sql.c_str(), sql.size());
	printf("libevent  mysql_send_query  %d\n", m->net.fd);
}

void query(const std::string& sql, mysql_context_t* c)
{
	c->index++;
	printf("query index %d\n", c->index);
	auto ret = mysql_send_query(c->m, sql.c_str(), sql.size());
	printf("mysql_send_query result %d\n", ret);
}


void test_libevent()
{
	printf("======== test_libevent ========\n");
	struct event_base* evbase = event_base_new();
	connect_mysql_server();
	query("select * from account;", conns[0]);
	query("select * from bag;", conns[1]);
	query("select * from account;", conns[2]);
	query("select * from bag;", conns[3]);
	bind_event(evbase);
	event_base_dispatch(evbase);
}


void bind_libuv(uv_loop_t* loop, mysql_context_t* context)
{
	uv_poll_init_socket(loop, &context->poll_handle, context->m->net.fd);
	context->poll_handle.data = context;
	uv_poll_start(&context->poll_handle , UV_READABLE   | UV_DISCONNECT, hand_libuv_event);
}

void test_libuv()
{
	printf("======== test_libuv ========\n");
	uv_loop_t* loop = uv_default_loop();

	mysql_context_t* c1 = new mysql_context_t;
	mysql_context_t* c2 = new mysql_context_t;
	mysql_context_t* c3 = new mysql_context_t;
	mysql_context_t* c4 = new mysql_context_t;

	connect_mysql_server(c1);
	bind_libuv(loop, c1);
	connect_mysql_server(c2);
	bind_libuv(loop, c2);
	connect_mysql_server(c3);
	bind_libuv(loop, c3);
	connect_mysql_server(c4);
	bind_libuv(loop, c4);

	query("select * from account;",c1->m);
	query("select * from bag;",c2->m);
	query("select * from account;",c3->m);
	query("select * from bag;",c4->m);

#if 0

	query("select * from account;",c2);
	query("select * from bag;",c2);
	query("select * from account;",c2);
	query("select * from bag;",c2);
#endif 
	int r = uv_run(loop, UV_RUN_DEFAULT);
}

#endif 




