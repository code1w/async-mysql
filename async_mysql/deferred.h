#pragma once
#include <functional>

namespace gamesh {
namespace mysql {

class Result;
class Deferred
{
private:
    std::function<void(Result&& result)> _successCallback;
    std::function<void(const char *error)> _failureCallback;
    std::function<void()> _completeCallback;
    bool requireStatus()
    {
        return _successCallback || _failureCallback;
    }

    void success(Result&& result)
    {
        if (_successCallback)   _successCallback(std::move(result));
        if (_completeCallback)  _completeCallback();
    }

    void failure(const char *error)
    {

        if (_failureCallback)   _failureCallback(error);
        if (_completeCallback)  _completeCallback();
    }

    void complete()
    {
        if (_completeCallback) _completeCallback();
    }
public:
    Deferred() {}
    Deferred(const Deferred& that) = delete;
    Deferred(Deferred&& that) = delete;
    Deferred& onSuccess(const std::function<void(Result&& result)>& callback)
    {
        _successCallback = callback;
        return *this;
    }
    Deferred& onFailure(const std::function<void(const char *error)>& callback)
    {
        _failureCallback = callback;
        return *this;
    }
    Deferred& onComplete(const std::function<void()>& callback)
    {
        _completeCallback = callback;
        return *this;
    }

    //friend class Connection;
    //friend class Statement;
};

}
}