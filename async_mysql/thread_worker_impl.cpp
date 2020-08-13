#include "thread_worker_impl.h"
namespace gamesh
{
    ThreadWorkerImpl::ThreadWorkerImpl()
        :thread_(&ThreadWorkerImpl::run, this)
    {
    }

    ThreadWorkerImpl::~ThreadWorkerImpl()
    {
        mutex_.lock();
        running_ = false;
        mutex_.unlock();

        condition_.notify_one();
        thread_.join();
    }


    void ThreadWorkerImpl::run()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (callbacks_.empty() && running_)
            {
                condition_.wait(lock);
            }

            if (callbacks_.empty()) {return;}

            auto callback = std::move(callbacks_.front());
            callbacks_.pop_front();
            lock.unlock();
            callback();
        }
    }

    // other thrad call this 
     void ThreadWorkerImpl::execute(const std::function<void()>& callback)
    {
        mutex_.lock();
        callbacks_.push_back(callback);
        mutex_.unlock();
        condition_.notify_one();
    }
}