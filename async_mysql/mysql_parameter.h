// Modified from REACT-CPP-MYSQL project 
#pragma once
#include <string>
#include <vector>
#include <mysql/mysql.h>
namespace gamesh { namespace mysql {
    class Parameter : public MYSQL_BIND
    {
    private:
        template <typename T>
        Parameter(enum_field_types type, T value)
        {
            memset(this, 0, sizeof(*this));
            buffer_type = type;
            is_unsigned = std::is_unsigned<T>::value;
            T *data = static_cast<T*>(std::malloc(sizeof(T)));
            *data = value;
            buffer = data;
        }
    public:
        Parameter(int8_t value)      : Parameter(MYSQL_TYPE_TINY,        value) {}
        Parameter(uint16_t value)    : Parameter(MYSQL_TYPE_SHORT,       value) {}
        Parameter(int16_t value)     : Parameter(MYSQL_TYPE_SHORT,       value) {}
        Parameter(uint32_t value)    : Parameter(MYSQL_TYPE_LONG,        value) {}
        Parameter(int32_t value)     : Parameter(MYSQL_TYPE_LONG,        value) {}
        Parameter(uint64_t value)    : Parameter(MYSQL_TYPE_LONGLONG,    value) {}
        Parameter(int64_t value)     : Parameter(MYSQL_TYPE_LONGLONG,    value) {}
        Parameter(float value)       : Parameter(MYSQL_TYPE_FLOAT,       value) {}
        Parameter(double value)      : Parameter(MYSQL_TYPE_DOUBLE,      value) {}
        Parameter(const std::string& value)
        {
            memset(this, 0, sizeof(*this));
            buffer_type = MYSQL_TYPE_STRING;
            char *data = static_cast<char*>(std::malloc(value.size()));
            std::memcpy(data, value.c_str(), value.size());
            buffer = data;
            buffer_length = value.size();
        }

        Parameter(const std::vector<char>& value)
        {
            memset(this, 0, sizeof(*this));
            buffer_type = MYSQL_TYPE_BLOB;
            char *data = static_cast<char*>(std::malloc(value.size()));
            std::memcpy(data, value.data(), value.size());
            buffer = data;
            buffer_length = value.size();
        }

        Parameter(std::nullptr_t value)
        {
            memset(this, 0, sizeof(*this));
            buffer_type = MYSQL_TYPE_NULL;
        }
        ~Parameter()
        {
             std::free(buffer);
        }
    };
}}
