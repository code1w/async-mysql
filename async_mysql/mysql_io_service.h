#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mysql/mysql.h>

#ifdef _WIN32
#include <Winsock2.h>
	typedef SOCKET fd_t;
#else
#include <sys/select.h>
	typedef int fd_t;
#endif 

#include "self_pipe.h"
#include "dll_export.h"

namespace gamesh
{
namespace mysql{

    typedef std::function<void(void* ud)> EventCallbackt;
    typedef std::function<void()> Taskt;

    class GAMESH_MYSQL_IOC_DLL_CLASS_DECL MysqlIOEvent
    {
    public:
        MysqlIOEvent(void)
            :rdcb_(nullptr)
            ,wrcb_(nullptr)
            ,fd(-1)
            ,ud(nullptr)
            ,isexerdcb_{false}
            ,isexewrcb_{false}
            ,marked_untrack_{false}
        {}

        fd_t fd;
        void* ud;
        EventCallbackt rdcb_;
        EventCallbackt wrcb_;
        std::atomic<bool> isexerdcb_;
        std::atomic<bool> isexewrcb_;
        std::atomic<bool> marked_untrack_;
    };

    

	class GAMESH_MYSQL_IOC_DLL_CLASS_DECL MysqlIOService
	{
	public:
        MysqlIOService(int threads);
		~MysqlIOService(void);
        MysqlIOService(const MysqlIOService&) = delete;
        MysqlIOService& operator=(const MysqlIOService&) = delete;

    public:
        void SetNBWorkers(int nb);

    public:
        void Track(fd_t fd, void* ud,  const EventCallbackt& rdcb, const EventCallbackt& wrcb);
        void SetReadCallback(fd_t fd, const EventCallbackt& event_callback);
        void SetWriteCallback(fd_t fd, const EventCallbackt& event_callback);
        void Untrack(fd_t fd);
        void WaitForRemoval(fd_t fd);
        void PostTask(const Taskt& task);
    private:
        void Poll();
        int  InitPollFdsEvent();
        void ProcessEvents();
        void ProcessReadEvent(const fd_t fd, MysqlIOEvent& event);
        void ProcessWriteEvent(const fd_t fd, MysqlIOEvent& event);
        void ExecuteTask();
    private:
        std::unordered_map<fd_t, MysqlIOEvent> tracked_events_;
        std::atomic<bool> stop_;
        std::thread poll_worker_;
        std::mutex event_mutex_;
        std::vector<fd_t> polled_fds_;
        fd_set readset_;
        fd_set writeset_;
        gamesh::SelfPipe notifier_;
        std::condition_variable wait_for_removal_condvar_;
        std::deque<Taskt> tasks_; // mysql query task
        std::mutex tasks_mutex_;

    };
}}
