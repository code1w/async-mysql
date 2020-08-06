#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdio>
#include <signal.h>
#include <mysql/mysql.h>
#include <event.h>
#include <uv.h>

typedef struct mysql_context_s {
	uv_poll_t poll_handle;
	MYSQL* m ;
	int index = 0;
	char name[32];

} mysql_context_t;

std::vector<MYSQL*> conns;
std::unordered_map<unsigned long long , event*> events;
void query(const std::string& sql,MYSQL *m);
void hand_libevent_event(intptr_t fd, short what, void* _userdata);
void hand_libuv_event(uv_poll_t* handle, int status, int what);
void mysql_write(MYSQL* m);

void mysql_read_result(MYSQL*);

void hand_libevent_event(intptr_t fd, short what, void* _userdata)
{
	/*
	printf("Got an event on socket %d:%s%s%s%s \n",
		(int) fd,
		(what&EV_TIMEOUT) ? " timeout" : "",
		(what&EV_READ)    ? " read" : "",
		(what&EV_WRITE)   ? " write" : "",
		(what&EV_SIGNAL)  ? " signal" : "");
	*/

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
	mysql_context_t* context = (mysql_context_t*)handle->data;
	
	/*
	printf("Got an uvevent on socket %d:%s%s%",
		(int) context->m->net.fd,
		(what&UV_READABLE)    ? " read" : "",
		(what&UV_WRITABLE)   ? " write" : "");
	*/

	printf("mysql_context_t index %d\n", context->index);

	if (what & UV_READABLE)
	{
		mysql_read_result(context->m);
	}
	if (what & UV_WRITABLE)
	{
		mysql_write(context->m);
	}

}

void mysql_write(MYSQL* m)
{
	//query("select * from bag;", m);
}

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
		printf("a result for m->net.fd = %d \n", m->net.fd);
		mysql_free_result(res);
	} else {
		std::cout <<" mysql_read_query_result() failed for fd = "<< m->net.fd << " "<<mysql_error(m)<< ":"<<mysql_errno(m)<< std::endl;
	}
	
}

void connect_mysql_server()
{
	for (int i = 0; i < 1; i++)
	{
		MYSQL* m = nullptr;
		m = mysql_init(nullptr);
		if (!m)
		{
			std::cout<<"mysql init error"<< std::endl;
			return;
		}
		m->free_me = 1;
		auto ret = mysql_real_connect(m,"192.168.150.79", "root","123456","gamesh_zxb",3306,NULL,0);
		if (!ret)
		{
			std::cout<<"Mysql conn err : "<<mysql_error(m) << ", errno :"<< mysql_errno(m)<< std::endl;
			return;
		}
		conns.push_back(m);
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
}

void bind_event(struct event_base* evbase)
{
	for (auto m : conns)
	{
		auto e = event_new(evbase, m->net.fd, EV_READ |EV_WRITE | EV_PERSIST, hand_libevent_event, m);
		event_add(e, NULL);
		events[m->net.fd] = e;
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
	struct event_base* evbase = event_base_new();
	connect_mysql_server();
	bind_event(evbase);
	query("select * from account;", conns[0]);
	query("select * from bag;", conns[0]);
	query("select * from account;", conns[0]);
	query("select * from bag;", conns[0]);
	event_base_dispatch(evbase);
}


void bind_libuv(uv_loop_t* loop, mysql_context_t* context)
{
	uv_poll_init_socket(loop, &context->poll_handle, context->m->net.fd);
	context->poll_handle.data = context;
	uv_poll_start(&context->poll_handle , UV_READABLE  | UV_WRITABLE | UV_DISCONNECT, hand_libuv_event);
}

void test_libuv()
{
	uv_loop_t* loop = uv_default_loop();

	mysql_context_t* c1 = new mysql_context_t;
	connect_mysql_server(c1);
	bind_libuv(loop, c1);
	mysql_context_t* c2 = new mysql_context_t;
	connect_mysql_server(c2);
	bind_libuv(loop, c2);

	query("select * from account;",c1->m);
	query("select * from bag;",c1->m);
	query("select * from account;",c1->m);
	query("select * from bag;",c1->m);

#if 0

	query("select * from account;",c2);
	query("select * from bag;",c2);
	query("select * from account;",c2);
	query("select * from bag;",c2);
#endif 
	int r = uv_run(loop, UV_RUN_DEFAULT);
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

	test_libevent();
	//test_libuv();
	return 0;
}