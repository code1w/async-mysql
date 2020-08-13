#pragma once

#include <mutex>
#include <thread>
#include <functional>
#include <deque>
#include <condition_variable>
#include "worker_impl.h"

namespace gamesh
{

#ifdef _WIN32
#include <Winsock2.h>
typedef SOCKET fd_t;
#else
#include <sys/select.h>
typedef int fd_t;
#endif 


class ThreadWorkerImpl : public WorkerImpl
{
private:
    volatile bool running_ = true;
    std::deque<std::function<void()>> callbacks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread thread_;
    void run();
public:
    ThreadWorkerImpl();
    virtual ~ThreadWorkerImpl();
    // other thrad call this 
    virtual void execute(const std::function<void()>& callback) override;

};

}

