#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mysql/mysql.h>
#include <memory>
#include <map>
#include "dll_export.h"
#include "mysql_result_impl.h"
#include "mysql_query_result_field_impl.h"

namespace gamesh { namespace mysql {

    class QueryResultImpl : public ResultImpl
    {
    private:
        MYSQL_RES *_result;
        std::vector<std::vector<std::unique_ptr<ResultFieldImpl>>> _rows;
        std::map<std::string, size_t> _fields;
        size_t _position;

    public:
        QueryResultImpl(MYSQL_RES *result) :
            ResultImpl(),
            _result(result),
            _rows(mysql_num_rows(_result)),
            _position(0)
        {
            auto size = mysql_num_fields(_result);
            for (size_t i = 0; i < size; i++)
            {
                auto field = mysql_fetch_field_direct(_result, i);
                _fields[std::string(field->name, field->name_length)] = i;
            }
        }
        virtual ~QueryResultImpl()
        {
            mysql_free_result(_result);
        }
        const std::map<std::string, size_t>& fields() const override
        {
            return _fields;
        }
        size_t size() const override
        {
            return _rows.size();
        }
        const std::vector<std::unique_ptr<ResultFieldImpl>>& fetch(size_t index) override
        {
            if (_rows[index].size() == 0)
            {
                if (_position != index)
                {
                     mysql_data_seek(_result, index);
                }
                _position = index + 1;

                auto row = mysql_fetch_row(_result);
                auto lengths = mysql_fetch_lengths(_result);
                _rows[index].reserve(_fields.size());
                for (size_t i = 0; i < _fields.size(); ++i)
                {
                    _rows[index].emplace_back(new QueryResultFieldImpl(row[i], lengths[i]));
                }
            }
            return _rows[index];
        }
    };

}}
