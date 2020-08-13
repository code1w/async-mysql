#pragma once
#include <string>
#include <vector>
#include <memory>
#include <ctime>

#include "dll_export.h"

namespace gamesh { namespace mysql {


    class GAMESH_MYSQL_IOC_DLL_CLASS_DECL ResultFieldImpl
    {
    public:
        virtual ~ResultFieldImpl() {}
        virtual bool isNULL() const = 0;
        virtual operator int8_t()    const = 0;
        virtual operator uint16_t()  const = 0;
        virtual operator int16_t()   const = 0;
        virtual operator uint32_t()  const = 0;
        virtual operator int32_t()   const = 0;
        virtual operator uint64_t()  const = 0;
        virtual operator int64_t()   const = 0;
        virtual operator float()     const = 0;
        virtual operator double()    const = 0;
        //virtual operator uint128_t() const = 0;
        virtual operator std::string() const = 0;
        virtual operator std::tm() const = 0;
    };
}}
