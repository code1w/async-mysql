#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mysql/mysql.h>
#include <memory>
#include "dll_export.h"
#include "mysql_result_field_impl.h"

namespace gamesh { namespace mysql {
    class ResultRowImpl;
    class ResultFieldImpl;

    class GAMESH_MYSQL_IOC_DLL_CLASS_DECL ResultImpl
    {
    public:
        virtual const std::map<std::string, size_t>& fields() const = 0;
        virtual size_t size() const = 0;
        virtual const std::vector<std::unique_ptr<ResultFieldImpl>>& fetch(size_t index) = 0;
    };
}}