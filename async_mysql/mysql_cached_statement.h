#pragma once
#include "dll_export.h"
#include <unordered_map>
#include <functional>
#include <mysql/mysql.h>
#include "mysql_io_service.h"
#include "worker.h"
#include "mysql_deferred.h"
#include "mysql_db.h"

namespace gamesh
{
	namespace mysql{
        class MysqlCachedStatement
        {
        private:
            Statement *_statement;
        public:
            MysqlCachedStatement(MysqlDB *db, const char *statement) :
                _statement(db->statement(statement))
            {}
            template <class ...Arguments>
            MysqlDeferred& execute(Arguments ...parameters)
            {
                return _statement->execute(std::forward<Arguments>(parameters)...);
            }
        };

}}