#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mysql/mysql.h>
#include <memory>
#include <map>
#include "dll_export.h"
#include "mysql_result_impl.h"
#include "mysql_result_field.h"
namespace gamesh { namespace mysql {
    class ResultImpl;
    class ResultField;

    class GAMESH_MYSQL_IOC_DLL_CLASS_DECL ResultRow
    {
    private:
        std::shared_ptr<ResultImpl> _result;
        const std::vector<std::unique_ptr<ResultFieldImpl>>& _fields;
        class GAMESH_MYSQL_IOC_DLL_CLASS_DECL iterator
        {
        private:
            std::map<std::string, size_t>::const_iterator _iterator;
            const ResultRow *_row;
        public:
             iterator() : _iterator(std::map<std::string, size_t>::const_iterator()), _row(NULL) {}
            iterator(std::map<std::string, size_t>::const_iterator&& iterator, const ResultRow *row) : _iterator(std::move(iterator)), _row(row) {}
            iterator(const iterator& that) : _iterator(that._iterator), _row(that._row) {}
            virtual ~iterator() {}
            iterator& operator=(const iterator& that)
            {
                _iterator = that._iterator;
                _row = that._row;

                return *this;
            }
            iterator &operator++()
            {
                _iterator++;
                return *this;
            }
            iterator operator++(int)
            {
                iterator copy(*this);
                _iterator++;
                return copy;
            }
            iterator &operator--()
            {
                _iterator--;
                return *this;
            }
            iterator operator--(int)
            {
                 iterator copy(*this);
                _iterator--;
                return copy;
            }
            bool operator==(const iterator& iterator) const
            {
                return _iterator == iterator._iterator;
            }
            bool operator!=(const iterator& iterator) const
            {
                return _iterator != iterator._iterator;
            }
            std::pair<std::string, ResultField> operator*() const
            {
                return std::make_pair<>(_iterator->first,_row->operator[](_iterator->first));
            }
            std::unique_ptr<std::pair<std::string, ResultField>> operator->() const
            {
                return std::unique_ptr<std::pair<std::string, ResultField>>(new std::pair<std::string, ResultField>(_iterator->first,_row->operator[](_iterator->first)));
            }
        };

    public:

        ResultRow(std::shared_ptr<ResultImpl> result, const std::vector<std::unique_ptr<ResultFieldImpl>>& fields) :
            _result(result), _fields(fields) {}

        virtual ~ResultRow() {}
         size_t size() const;
        const ResultField operator [] (size_t index) const;
        const ResultField operator [] (const std::string &key) const;
        iterator begin() const;
        iterator find(const std::string& key) const;
        iterator end() const;
    };


}}
