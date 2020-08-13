#include "self_pipe.h"
#include "typedefs.h"
#include <fcntl.h>
#include <iostream>
#include <assert.h>


#ifndef _WIN32
#include <unistd.h>
#endif
namespace gamesh
{
#ifdef _WIN32
    SelfPipe::SelfPipe()
        : fd_(INVALID_FD) 
    {
        fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        assert(fd_ != INVALID_FD);
        u_long flags = 1;
        ioctlsocket(fd_, FIONBIO, &flags);
        struct sockaddr_in inaddr;
        memset(&inaddr, 0, sizeof(inaddr));
        inaddr.sin_family      = AF_INET;
        inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        inaddr.sin_port        = 0;
        int r =bind(fd_, (struct sockaddr*) &inaddr, sizeof(inaddr));
        assert(r != SOCKET_ERROR);
        addrlen_ = sizeof(addr_);
        memset(&addr_, 0, sizeof(addr_));
        r = getsockname(fd_, &addr_, &addrlen_);
        assert(r != SOCKET_ERROR);
        r = connect(fd_, &addr_, addrlen_) ;
        assert(r!= SOCKET_ERROR);
    }

    SelfPipe::~SelfPipe() 
    {
        if (fd_ != INVALID_FD) 
        {
            closesocket(fd_);
        }
    }

    fd_t SelfPipe::GetReadFd() const 
    {
        return fd_;
    }

    fd_t SelfPipe::GetWriteFd() const 
    {
        return fd_;
    }

    void SelfPipe::Notify() 
    {
        (void) sendto(fd_, "a", 1, 0, &addr_, addrlen_);
    }

    void SelfPipe::ClrBuffer() 
    {
        char buf[1024];
        (void) recvfrom(fd_, buf, 1024, 0, &addr_, &addrlen_);
    }
#else 
    SelfPipe::SelfPipe()
    {
        if (pipe(fds_) == -1) { }
    }

    SelfPipe::~SelfPipe() {
        if (fds_[0] != __TACOPIE_INVALID_FD) {
            close(fds_[0]);
        }

        if (fds_[1] != __TACOPIE_INVALID_FD) {
            close(fds_[1]);
        }
    }


    fd_t SelfPipe::GetReadFd() const {
        return fds_[0];
    }

    fd_t SelfPipe::GetWriteFd() const {
        return fds_[1];
    }


    void SelfPipe::Notify() {
        (void) write(fds_[1], "a", 1);
    }


    void SelfPipe::ClrBuffer() {
        char buf[1024];
        (void) read(fds_[0], buf, 1024);
    }
#endif 

}
