#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mysql/mysql.h>
#include <memory>
#include <map>
#include <ctime>
#include "dll_export.h"

namespace gamesh { namespace mysql {
    class ResultImpl;
    class ResultFieldImpl;
    class GAMESH_MYSQL_IOC_DLL_CLASS_DECL ResultField
    {
    private:
        std::shared_ptr<ResultImpl> _result;
        ResultFieldImpl *_field;
    public:
        ResultField(std::shared_ptr<ResultImpl> result, ResultFieldImpl *field);
        bool isNULL() const;
        operator int8_t()   const;
        operator uint16_t() const;
        operator int16_t()  const;
        operator uint32_t() const;
        operator int32_t()  const;
        operator uint64_t() const;
        operator int64_t()  const;
        operator float()    const;
        operator double()   const;
        //operator uint128_t() const;
        operator std::string() const;
        operator std::tm() const;
    };
    GAMESH_MYSQL_IOC_DLL_DECL std::ostream& operator<<(std::ostream& stream, const ResultField& field);
}}
