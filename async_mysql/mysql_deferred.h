// Modified from REACT-CPP-MYSQL project 
#pragma once
#include <mutex>
#include <thread>
#include <functional>
#include <deque>
#include <condition_variable>
#include <iostream>
#include "dll_export.h"


namespace gamesh
{
namespace mysql
{
    class Connection;
    class Result;

    class GAMESH_MYSQL_IOC_DLL_CLASS_DECL Deferred
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
        ~Deferred(){std::cout<<"~Deferred"<<std::endl;}
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
        friend class MysqlDB;
    };
}}
