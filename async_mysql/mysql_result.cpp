
#include "mysql_result.h"
#include "mysql_result_row.h"
#include "mysql_query_result_field_impl.h"
#include "mysql_query_result_impl.h"

namespace gamesh { namespace mysql {
    Result::Result(MYSQL_RES *result) :
        _result(std::make_shared<QueryResultImpl>(result))
    {}
    Result::Result(std::shared_ptr<ResultImpl>&& implementation) :
        _result(std::move(implementation))
    {}

    Result::Result(size_t affectedRows, uint64_t insertID) :
        _affectedRows(affectedRows),
        _insertID(insertID)
    {}

    Result::Result(std::nullptr_t result)
    {}

     Result::Result(Result&& that)
         : _result(std::move(that._result))
        ,_affectedRows(that._affectedRows)
    {}
    Result::~Result()
    {}

    Result::iterator::iterator() :
        _result(nullptr),
        _index(0)
    {}
    Result::iterator::iterator(std::shared_ptr<ResultImpl> result, size_t index) :
        _result(result),
        _index(index)
    {}

    Result::iterator::iterator(const Result::iterator& that) :
        _result(that._result),
        _index(that._index)
    {}
    bool Result::iterator::valid() const
    {
        return _result && _index < _result->size();
    }
    Result::iterator& Result::iterator::operator=(const iterator& that)
    {
        _result = that._result;
        _index = that._index;
        return *this;
    }

    Result::iterator& Result::iterator::operator++()
    {
        ++_index;
        return *this;
    }
    Result::iterator Result::iterator::operator++(int)
    {
        iterator copy(*this);

        ++_index;
        return copy;
    }

    bool Result::iterator::operator==(const iterator& that)
    {
        if (_result != that._result) return false;

        if (!valid() && !that.valid()) return true;

        return _index == that._index;
    }
    bool Result::iterator::operator!=(const iterator& that)
    {
        return !operator==(that);
    }

    ResultRow Result::iterator::operator*()
    {
        return ResultRow(_result, _result->fetch(_index));
    }

    std::unique_ptr<ResultRow> Result::iterator::operator->()
    {
        return std::unique_ptr<ResultRow>(new ResultRow(_result, _result->fetch(_index)));
    }

    bool Result::valid() const
    {
        return _affectedRows || _result;
    }

    size_t Result::affectedRows() const
    {
        return _affectedRows;
    }

    uint64_t Result::insertID() const
    {
        return _insertID;
    }

    size_t Result::size() const
    {
        return _result ? _result->size() : 0;
    }

    ResultRow Result::operator [] (size_t index)
    {
       // if (!_result) throw Exception("Invalid result object");
        return ResultRow(_result, _result->fetch(index));
    }


    Result::iterator Result::begin() const
    {
        return iterator(_result, 0);
    }

    Result::iterator Result::end() const
    {
        return iterator(_result, _result->size());
    }
}}
