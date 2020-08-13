
#include <string>
#include <vector>
#include <mysql/mysql.h>
#include <memory>
#include "dll_export.h"

namespace gamesh { namespace mysql {
    class ResultImpl;
    class ResultRow;
    class GAMESH_MYSQL_IOC_DLL_CLASS_DECL Result
    {
        std::shared_ptr<ResultImpl> _result;
        size_t _affectedRows = 0;
        uint64_t _insertID = 0;

    public:
        class GAMESH_MYSQL_IOC_DLL_CLASS_DECL iterator
        {
        private:
            std::shared_ptr<ResultImpl> _result;

            size_t _index;
            bool valid() const;
        public:
            iterator();

            iterator(std::shared_ptr<ResultImpl> result, size_t index);
            iterator(const iterator& that);
            iterator& operator=(const iterator& that);
            iterator& operator++();
            iterator operator++(int);
            bool operator==(const iterator& that);
            bool operator!=(const iterator& that);
            ResultRow operator*();
            std::unique_ptr<ResultRow> operator->();
        };

        Result(MYSQL_RES *result);
        Result(std::shared_ptr<ResultImpl>&& implementation);
        Result(size_t affectedRows, uint64_t insertID);
        Result(std::nullptr_t result);
        Result(Result&& that);
        Result(const Result& that) = delete;
        virtual ~Result();
        bool valid() const;
        size_t affectedRows() const;
        uint64_t insertID() const;
        size_t size() const;
        ResultRow operator [] (size_t index);
        iterator begin() const;
        iterator end() const;
    };
}}
