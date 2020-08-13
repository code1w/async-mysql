
#include "mysql_result_row.h"

namespace gamesh { namespace mysql {

    size_t ResultRow::size() const
    {
        return _result->fields().size();
    }
    const ResultField ResultRow::operator [] (size_t index) const
    {
        //if (index >= size()) throw Exception("Index out of bounds");

        return ResultField(_result, _fields[index].get());
    }

    const ResultField ResultRow::operator [] (const std::string &key) const
    {
        auto iter = _result->fields().find(key);
        //if (iter == _result->fields().end()) throw Exception("Field key does not exist");
        return ResultField(_result, _fields[iter->second].get());
    }

    ResultRow::iterator ResultRow::begin() const
    {
        return iterator(_result->fields().cbegin(), this);
    }

    ResultRow::iterator ResultRow::find(const std::string& key) const
    {
        return iterator(_result->fields().find(key), this);
    }
    ResultRow::iterator ResultRow::end() const
    {
        return iterator(_result->fields().cend(), this);
    }

}}
