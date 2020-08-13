#include "mysql_result_field.h"
#include "mysql_result_field_impl.h"
#include "mysql_result_impl.h"

#include <iostream>


namespace gamesh { namespace mysql {

    ResultField::ResultField(std::shared_ptr<ResultImpl> result, ResultFieldImpl *field) :
        _result(result),
        _field(field)
    {}


    bool ResultField::isNULL() const
    {
        return _field == nullptr || _field->isNULL();
    }
    ResultField::operator int8_t()   const { return *_field; }
    ResultField::operator uint16_t() const { return *_field; }
    ResultField::operator int16_t()  const { return *_field; }
    ResultField::operator uint32_t() const { return *_field; }
    ResultField::operator int32_t()  const { return *_field; }
    ResultField::operator uint64_t() const { return *_field; }
    ResultField::operator int64_t()  const { return *_field; }
    ResultField::operator float()    const { return *_field; }
    ResultField::operator double()   const { return *_field; }

    //ResultField::operator uint128_t() const { return *_field; }
    ResultField::operator std::string() const
    {
        return *_field;
    }
    ResultField::operator std::tm() const
    {
        return *_field;
    }

    std::ostream& operator<<(std::ostream& stream, const ResultField& field)
    {
        if (field.isNULL()) return stream << "(NULL)";
        else return stream << (std::string) field;
    }
}}
