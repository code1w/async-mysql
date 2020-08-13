#include "mysql_io_service.h"
#include <cstdio>

namespace gamesh
{
namespace mysql{

        MysqlIOService::MysqlIOService(int nb_threads)
            : stop_(false)
        {
            poll_worker_ = std::thread(std::bind(&MysqlIOService::Poll, this));
        }

        MysqlIOService::~MysqlIOService(void) 
        {
            stop_ = true;
            notifier_.Notify();
            if (poll_worker_.joinable()) {
                poll_worker_.join();
            }
            //m_callback_workers.stop();
        }

        int MysqlIOService::InitPollFdsEvent(void) {

            polled_fds_.clear();
            FD_ZERO(&readset_);
            FD_ZERO(&writeset_);

            int ndfs = (int) notifier_.GetReadFd();
            FD_SET(notifier_.GetReadFd(), &readset_);
            polled_fds_.push_back(notifier_.GetReadFd());

            std::lock_guard<std::mutex> lock(event_mutex_);
            for (const auto& it : tracked_events_) {
                const auto& fd = it.first;
                const auto& evnt = it.second;

                bool should_rd = evnt.rdcb_ && !evnt.isexerdcb_;
                if (should_rd) {
                    FD_SET(fd, &readset_);
                }

                bool should_wr = evnt.wrcb_ && !evnt.isexewrcb_;
                if (should_wr) {
                    FD_SET(fd, &writeset_);
                }

                if (should_rd || should_wr || evnt.marked_untrack_) {
                    polled_fds_.push_back(fd);
                }

                if ((should_rd || should_wr) && (int) fd > ndfs) {
                    ndfs = (int) fd;
                }
            }

            return ndfs + 1;
        }

        void MysqlIOService::Poll(void) 
        {
            while (!stop_) 
            {
                int ndfs = InitPollFdsEvent();
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 10000; //  max wait time 10ms
                int ret = select(ndfs, &readset_, &writeset_, nullptr, &tv);
                if (ret > 0) 
                {
                    ProcessEvents();
                    ExecuteTask();
                }
                else 
                {
                    // timer tick
                }
            }
        }

        void MysqlIOService::ProcessEvents(void) 
        {
            std::lock_guard<std::mutex> lock(event_mutex_);
            for (const auto& fd : polled_fds_) {
                if (fd == notifier_.GetReadFd() && FD_ISSET(fd, &readset_)) {
                    notifier_.ClrBuffer();
                    continue;
                }

                auto it = tracked_events_.find(fd);

                if (it == tracked_events_.end()) { continue; }

                auto& ioevent = it->second;

                if (FD_ISSET(fd, &readset_) && ioevent.rdcb_ && !ioevent.isexerdcb_) {
                    ProcessReadEvent(fd, ioevent);
                }
                if (FD_ISSET(fd, &writeset_) && ioevent.wrcb_ && !ioevent.isexewrcb_) {
                    ProcessWriteEvent(fd, ioevent);
                }

                if (ioevent.marked_untrack_ && !ioevent.isexerdcb_ && !ioevent.isexewrcb_) {
                    tracked_events_.erase(it);
                    wait_for_removal_condvar_.notify_all();
                }
            }
        }

        void  MysqlIOService::ProcessReadEvent(const fd_t fd, MysqlIOEvent& ioevent) {

            auto readcb = ioevent.rdcb_;
            ioevent.isexerdcb_.store(true);
            readcb(ioevent.ud);
            ioevent.isexerdcb_.store(false);

            auto it = tracked_events_.find(fd);

            if (it == tracked_events_.end()) { return; }

            if (it->second.marked_untrack_ && !it->second.isexewrcb_) {
                tracked_events_.erase(it);
                wait_for_removal_condvar_.notify_all();
            }
            notifier_.Notify();
            
        }

        void  MysqlIOService::ProcessWriteEvent(const fd_t fd, MysqlIOEvent& ioevent) {
            auto writecb = ioevent.wrcb_;
            ioevent.isexerdcb_.store(true);
            writecb(ioevent.ud);
            ioevent.isexerdcb_.store(false);
            auto it = tracked_events_.find(fd);
            if (it == tracked_events_.end()) { return; }
            if (it->second.marked_untrack_ && !it->second.isexerdcb_) 
            {
                tracked_events_.erase(it);
                wait_for_removal_condvar_.notify_all();
            }
            notifier_.Notify();
        }

        void MysqlIOService::Track(fd_t fd, void* ud , const EventCallbackt& rdcb, const EventCallbackt& wrcb) 
        {
            std::lock_guard<std::mutex> lock(event_mutex_);
            auto& event = tracked_events_[fd];
            event.rdcb_ = rdcb;
            event.wrcb_ = wrcb;
            event.ud = ud;
            event.marked_untrack_.store(false);
            notifier_.Notify();
        }

        void MysqlIOService::PostTask(const Taskt& task)
        {
            std::lock_guard<std::mutex> l(tasks_mutex_);
            tasks_.push_back(task);
            notifier_.Notify();
        }

        void MysqlIOService::ExecuteTask()
        {
            if (tasks_.empty())
            {
                return;
            }

            Taskt task = nullptr;
            {
                std::unique_lock<std::mutex> lock(tasks_mutex_);
                task = std::move(tasks_.front());
                tasks_.pop_front();
                lock.unlock();
            }
            if (task)
            {
                task();
            }
        }

        void  MysqlIOService::SetReadCallback(fd_t fd, const EventCallbackt& cb)
        {
            std::lock_guard<std::mutex> lock(event_mutex_);
            auto& event  = tracked_events_[fd];
            event.rdcb_ = cb;
            notifier_.Notify();
        }

        void MysqlIOService::SetWriteCallback(fd_t fd, const EventCallbackt& cb)
        {
            std::lock_guard<std::mutex> lock(event_mutex_);
            auto& event  = tracked_events_[fd];
            event.wrcb_ = cb;
            notifier_.Notify();
        }

        void MysqlIOService::Untrack(fd_t fd) 
        {
            std::lock_guard<std::mutex> lock(event_mutex_);
            auto it = tracked_events_.find(fd);

            if (it == tracked_events_.end()) { return; }

            if (it->second.isexerdcb_ || it->second.isexewrcb_) {
                it->second.marked_untrack_.store(true);
            }
            else {
                tracked_events_.erase(it);
                wait_for_removal_condvar_.notify_all();
            }

            notifier_.Notify();
        }

        void  MysqlIOService::WaitForRemoval(fd_t fd) {
            std::unique_lock<std::mutex> lock(event_mutex_);
            wait_for_removal_condvar_.wait(lock, [&]() {
                return tracked_events_.find(fd) == tracked_events_.end();
            });
        }
        
        void MysqlIOService::SetNBWorkers(int nb)
        {

        }
	}}