#pragma once
#include "typedefs.h"

namespace gamesh
{
	class SelfPipe {
	public:
		SelfPipe();
		~SelfPipe();
		SelfPipe(const SelfPipe&) = delete;
		SelfPipe& operator=(const SelfPipe&) = delete;

	public:
		fd_t GetReadFd() const;
		fd_t GetWriteFd() const;
		void Notify();
		void ClrBuffer();

	private:
#ifdef _WIN32
		fd_t fd_;
		struct sockaddr addr_;
		int addrlen_;
#else
		fd_t fds_[2]={INVALID_FD, INVALID_FD};
#endif 
	};
}