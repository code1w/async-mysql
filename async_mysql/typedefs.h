#pragma once
#ifdef _WIN32
#include <Winsock2.h>
#endif /* _WIN32 */

namespace gamesh
{
#ifdef _WIN32
	typedef SOCKET fd_t;
#define INVALID_FD INVALID_SOCKET
#else
	typedef int fd_t;
#define INVALID_FD -1
#endif /* _WIN32 */
}