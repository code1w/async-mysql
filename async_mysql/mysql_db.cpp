#include "mysql_db.h"
#include "mysql_local_parameter.h"
#include "mysql_result.h"
#include <cstdio>

namespace gamesh { namespace mysql {

    static void init()
    {
        static Library library;
    }

    MysqlDB::MysqlDB(MysqlIOService *loop, bool initialize, int pollsize) :
        loop_(loop),
        mysql_(nullptr),
        master_(),
        worker_()
    {
        if (initialize)
        {
            init();
        }
        myslqpoll_.resize(pollsize);
        myslqpoll_.reserve(pollsize);
    }

    MysqlDB::~MysqlDB()
    {
        worker_.execute([this]() {
            if (mysql_) mysql_close(mysql_);
            mysql_thread_end();
        });
    }


    void MysqlDB::syncConnect(const std::string& hostname, const std::string& username, const std::string& password, const std::string& database, uint32_t port , uint64_t flags )
    {
       loop_->PostTask([this, hostname, username, password, database, port, flags]() {
        if ((mysql_ = mysql_init(nullptr)) == nullptr)
        {
            //master_.execute([this]() { if (connectCallback_)  connectCallback_(mysql_error(mysql_)); });
            loop_->PostTask([this]() { if (connectCallback_)  connectCallback_(mysql_error(mysql_)); });
            return;
        }

        my_bool reconnect = 1;
        mysql_options(mysql_, MYSQL_OPT_RECONNECT, &reconnect);
        if (mysql_real_connect(mysql_, hostname.c_str(), username.c_str(), password.c_str(), database.c_str(), port, nullptr, flags) == nullptr)
        {
            //master_.execute([this]() { if (connectCallback_) connectCallback_(mysql_error(mysql_)); });
            loop_->PostTask([this]() { if (connectCallback_) connectCallback_(mysql_error(mysql_)); });
            return;
        }
        else
        {
            loop_->Track(mysql_->net.fd, mysql_, nullptr, nullptr);
        }
        //master_.execute([this]() { if (connectCallback_) connectCallback_(nullptr); });
        loop_->PostTask([this]() { if (connectCallback_) connectCallback_(nullptr); });
        
        });


#if 0
        worker_.execute([this, hostname, username, password, database, port, flags]() {
            if ((mysql_ = mysql_init(nullptr)) == nullptr)
            {
                master_.execute([this]() { if (connectCallback_)  connectCallback_(mysql_error(mysql_)); });
                return;
            }

            my_bool reconnect = 1;
            mysql_options(mysql_, MYSQL_OPT_RECONNECT, &reconnect);
            if (mysql_real_connect(mysql_, hostname.c_str(), username.c_str(), password.c_str(), database.c_str(), port, nullptr, flags) == nullptr)
            {
                master_.execute([this]() { if (connectCallback_) connectCallback_(mysql_error(mysql_)); });
                return;
            }
            master_.execute([this]() { if (connectCallback_) connectCallback_(nullptr); });
        });
#endif 

    }

    void MysqlDB::onConnected(const std::function<void(const char *error)>& callback)
    {
        connectCallback_ = callback;
    }

    void MysqlDB::prepare(const std::string& query, LocalParameter *parameters, size_t count, const std::function<void(const std::string& query)>& callback)
    {

        worker_.execute([this, callback, query, parameters, count] () {
            size_t size = query.size();
            for (size_t i = 0; i < count; ++i) size += parameters[i].size();
            std::string result;
            result.reserve(size);
            size_t position = query.find_first_of("?!");
            result.append(query, 0, position);
            for (size_t i = 0; i < count; ++i)
            {
                if (position == std::string::npos) break;
                auto &parameter = parameters[i];
                switch (query[position])
                {
                case '?':
                    result.append(parameter.quote(mysql_));
                    break;
                case '!':
                    result.append(parameter.escape(mysql_));
                    break;
                }
                size_t next = query.find_first_of("?!", position + 1);
                result.append(query, position + 1, next - position - 1);
                position = next;
            }
            delete [] parameters;
            master_.execute([ callback, result]() { callback(result); });
            });
    }

    Deferred& MysqlDB::query(const std::string& query)
    {
        auto deferred = std::make_shared<Deferred>();

        auto resultcb = [this, deferred](void* ud){
            if (!mysql_read_query_result(mysql_))
            {
                while (true)
                {
                    auto *result = mysql_store_result(mysql_);
                    if (!deferred->requireStatus())
                    {
                        if (result) mysql_free_result(result);
                    }
                    else
                    {
                        size_t affectedRows = mysql_affected_rows(mysql_);
                        if (result)
                        {
                            //master_.execute([this, deferred, result]() { deferred->success(Result(result)); });
                            loop_->PostTask([this, deferred, result]() { deferred->success(Result(result)); });
                        }
                        else if (mysql_field_count(mysql_))
                        {
                            //master_.execute([this, deferred]() { deferred->failure(mysql_error(mysql_)); });
                            loop_->PostTask([this, deferred]() { deferred->failure(mysql_error(mysql_)); });
                        }
                        else
                        {
                            //master_.execute([this, deferred, affectedRows]() { deferred->success(Result(affectedRows, mysql_insert_id(mysql_))); });
                            loop_->PostTask([this, deferred, affectedRows]() { deferred->success(Result(affectedRows, mysql_insert_id(mysql_))); });
                        }
                    }
                    switch(mysql_next_result(mysql_))
                    {
                    case -1:
                        return;
                    case 0:
                        continue;
                    default:
                        master_.execute([this, deferred]() { deferred->failure(mysql_error(mysql_)); });
                        return;
                    }
                }
            }
        };
       
        printf("Result Cb %p \n",resultcb);

        loop_->PostTask([this, query, deferred, resultcb]() {
            loop_->Track(mysql_->net.fd, mysql_, resultcb, nullptr);
            if (mysql_send_query(mysql_, query.c_str(),query.size()))
            {
                if (deferred->requireStatus()) master_.execute([this, deferred]() { deferred->failure(mysql_error(mysql_)); });
                return;
            }});
#if 0
        worker_.execute([this, query, deferred]() {
            if (mysql_query(mysql_, query.c_str()))
            {
                if (deferred->requireStatus()) master_.execute([this, deferred]() { deferred->failure(mysql_error(mysql_)); });
                return;
            }
            while (true)
            {
                auto *result = mysql_store_result(mysql_);
                if (!deferred->requireStatus())
                {
                    if (result) mysql_free_result(result);
                }
                else
                {
                    size_t affectedRows = mysql_affected_rows(mysql_);
                    if (result)
                    {
                        master_.execute([this, deferred, result]() { deferred->success(Result(result)); });
                    }
                    else if (mysql_field_count(mysql_))
                    {
                        master_.execute([this, deferred]() { deferred->failure(mysql_error(mysql_)); });
                    }
                    else
                    {
                        master_.execute([this, deferred, affectedRows]() { deferred->success(Result(affectedRows, mysql_insert_id(mysql_))); });
                    }
                }
                switch(mysql_next_result(mysql_))
                {
                case -1:
                    return;
                case 0:
                    continue;
                default:
                    master_.execute([this, deferred]() { deferred->failure(mysql_error(mysql_)); });
                    return;
                }
            }
            });
#endif 
        return *deferred;
    }


    /*
    Statement *Connection::statement(const char *query)
    {
    // find a possibly existing statement
    auto iter = _statements.find(query);

    // do we already have this statement?
    if (iter != _statements.end()) return iter->second.get();

    // create a new statement and store it
    auto *statement = new Statement(this, query);
    _statements[query].reset(statement);

    // return the newfangled statement
    return statement;
    }
    */


}}
