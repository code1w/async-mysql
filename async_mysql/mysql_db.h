// Modified from REACT-CPP-MYSQL project 
#pragma once
#include "dll_export.h"
#include <unordered_map>
#include <functional>
#include <mysql/mysql.h>
#include "mysql_io_service.h"
#include "worker.h"
#include "mysql_deferred.h"

namespace gamesh { namespace mysql {

    class LocalParameter;

    class Library
    {
    public:
        Library()
        {
            mysql_library_init(0, nullptr, nullptr);
        }
        ~Library()
        {
            mysql_library_end();
        }
    };

	class GAMESH_MYSQL_IOC_DLL_CLASS_DECL MysqlDB
	{

    private:
        MysqlIOService *loop_;
        MYSQL* mysql_;
        std::function<void(const char *error)> connectCallback_;
        //std::unordered_map<const char *, std::unique_ptr<Statement>> _statements;
        Worker master_;
        Worker worker_;
        std::vector<MYSQL*> myslqpoll_; //

       // Statement *statement(const char *query);
        void prepare(const std::string& query, LocalParameter *parameters, size_t count, const std::function<void(const std::string& query)>& callback);
    public:
        MysqlDB(MysqlIOService *loop, bool initialize = true, int pollsize = 1);
        virtual ~MysqlDB();
        void syncConnect(const std::string& hostname, const std::string &username, const std::string& password, const std::string& database, uint32_t port = 3306, uint64_t flags = CLIENT_IGNORE_SIGPIPE | CLIENT_MULTI_STATEMENTS);
        void onConnected(const std::function<void(const char *error)>& callback);
        Deferred& query(const std::string& query);

        template <class ...Arguments>
        Deferred& execute(const std::string& query, Arguments ...parameters)
        {
            if (sizeof...(parameters) == 0) return this->query(query);
            auto deferred = std::make_shared<Deferred>();
            prepare(query, new LocalParameter[sizeof...(parameters)]{ parameters... }, sizeof...(parameters), [this, deferred](const std::string& query) {
                auto &result = this->query(query);
                result.onComplete([deferred]() { deferred->complete(); });
                if (deferred->requireStatus())
                {
                    result.onSuccess([deferred](Result&& result)   { deferred->success(std::move(result)); });
                    result.onFailure([deferred](const char *error) { deferred->failure(error); });
                }
                });
            return *deferred;
        }
        friend class Statement;
        friend class MysqlCachedStatement;
	};
}}