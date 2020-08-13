// Modified from REACT-CPP-MYSQL project 
#pragma once
#include <string>
#include <mysql/mysql.h>

namespace gamesh { namespace mysql {
    class LocalParameter
    {
    private:
        std::string _value;
        bool _integral;
        char *_buffer;
    public:
        LocalParameter(uint8_t value)     : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(int8_t value)      : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(uint16_t value)    : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(int16_t value)     : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(uint32_t value)    : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(int32_t value)     : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(uint64_t value)    : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(int64_t value)     : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(float value)       : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(double value)      : _value(std::to_string(value)), _integral(true), _buffer(nullptr) {}
        LocalParameter(const std::string& value) :
            _value(value),
            _integral(false),
            _buffer(static_cast<char*>(std::malloc(size())))
        {}
        LocalParameter(const char *value) :
            _value(value),
            _integral(false),
            _buffer(static_cast<char*>(std::malloc(size())))
        {}
        LocalParameter(std::nullptr_t value) :
            _value("NULL"),
            _integral(true),
            _buffer(nullptr)
        {}
        virtual ~LocalParameter()
        {
            std::free(_buffer);
        }
        size_t size() const
        {
            if (_integral) return _value.size();
            else return _value.size() * 2 + 2;
        }
        const std::string escape(MYSQL *connection)
        {
            if (_integral) return _value;
            auto length = mysql_real_escape_string(connection, _buffer, _value.c_str(), _value.size());
            return std::string(_buffer, length);
        }


        const std::string quote(MYSQL *connection)
        {

            if (_integral) return _value;
            _buffer[0] = '\'';
            auto length = mysql_real_escape_string(connection, _buffer + 1, _value.c_str(), _value.size());
            _buffer[length + 1] = '\'';
            _buffer[length + 2] = '\0';
            return std::string(_buffer, length + 2);
        }
    };

}}
