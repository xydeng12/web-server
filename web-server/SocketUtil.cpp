#include "SocketUtil.h"
#include "base/Logger.h"

#include <sys/socket.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

void setNonBlockAndCloseOnExec(int sockfd)
{
	int flags;
	if((flags = ::fcntl(sockfd, F_GETFL, 0)) < 0 )
	{
		LOG_ERROR << "fcntl F_GETFL";
	}
	flags |= O_NONBLOCK;
	if(::fcntl(sockfd, F_SETFL, flags) < 0)
	{
		LOG_ERROR << "fcntl F_SETFL O_NONBLOCK";
	}
	
	if((flags = ::fcntl(sockfd, F_GETFL, 0)) < 0 )
	{
		LOG_ERROR << "fcntl F_GETFL";
	}
	flags |= FD_CLOEXEC;
	if(::fcntl(sockfd, F_SETFL, flags) < 0)
	{
		LOG_ERROR << "fcntl F_SETFL FD_CLOEXEC";
	}
}

int SocketUtil::createSocket(int family)
{
	int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0)
	{
		LOG_FATAL << "SocketUtil::createSocket";
	}
	setNonBlockAndCloseOnExec(sockfd);
	return sockfd;
}

void SocketUtil::bind(int sockfd, const struct sockaddr* addr)
{
	int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
	if(ret < 0)
	{
		LOG_FATAL << "SocketUtil::bind";
	}
}

void SocketUtil::listen(int sockfd)
{
	int ret = ::listen(sockfd, SOMAXCONN);
	if(ret < 0)
	{
		LOG_FATAL << "SocketUtil::listen";
	}
}

int SocketUtil::accept(int sockfd, struct sockaddr_in* addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr));
	int connfd = ::accept(sockfd, reinterpret_cast<struct sockaddr*>(addr), &addrlen);
	if(connfd >= 0)
	{
		setNonBlockAndCloseOnExec(connfd);
	}
	else
	{
		LOG_ERROR << "SocketUtil::accept" 
				  << " errno = " << errno;		
	}
	return connfd;
}

int SocketUtil::connect(int sockfd, const struct sockaddr* servaddr)
{
	return ::connect(sockfd, servaddr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

ssize_t SocketUtil::read(int sockfd, void *buf, size_t count)
{
	return ::read(sockfd, buf, count);
}

ssize_t SocketUtil::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
	return ::readv(sockfd, iov, iovcnt);
}

ssize_t SocketUtil::write(int sockfd, const void *buf, size_t count)
{
	return ::write(sockfd, buf, count);
}

ssize_t SocketUtil::writev(int sockfd, const struct iovec *iov, int iovcnt)
{
	return ::writev(sockfd, iov, iovcnt);
}

void SocketUtil::close(int sockfd)
{
	LOG_INFO << "socket fd = " << sockfd << " closed";
	if(::close(sockfd) < 0)
	{
		LOG_ERROR << "SocketUtil::close";
	}
}

void SocketUtil::shutdownWrite(int sockfd)
{
	if(::shutdown(sockfd, SHUT_WR) < 0)
	{
		LOG_ERROR << "SocketUtil::shutdownWrite";
	}
}

void SocketUtil::IpPortToSockaddr(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	int ret = inet_pton(AF_INET, ip, &addr->sin_addr);
	if(ret == 0)
	{
		LOG_ERROR << "SocketUtil::IpPortToSockaddr : input invalid";
	}
	else if(ret < 0)
	{
		LOG_ERROR << "SocketUtil::IpPortToSockaddr : error";	
	}
}

void SocketUtil::SockaddrToIp(char* buf, size_t size, const struct sockaddr* addr)
{
	if(addr->sa_family == AF_INET)
	{
		assert(size >= INET_ADDRSTRLEN);
		const struct sockaddr_in* addr = reinterpret_cast<const struct sockaddr_in*>(addr);
		::inet_ntop(AF_INET, &addr->sin_addr, buf, size);
	}
}

struct sockaddr_in SocketUtil::getLocalAddress(int sockfd)
{
	struct sockaddr_in localaddr;
	bzero(&localaddr, sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
	if(::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen) < 0)
	{
		LOG_ERROR << "SocketUtil::getLocalAddress";
	}
	return localaddr;
}

struct sockaddr_in SocketUtil::getPeerAddress(int sockfd)
{
	struct sockaddr_in peeraddr;
	bzero(&peeraddr, sizeof peeraddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
	if(::getpeername(sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr), &addrlen) < 0)
	{
		LOG_ERROR << "SocketUtil::getPeerAddress";
	}
	return peeraddr;
}

int SocketUtil::getSocketError(int sockfd)
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
	{
		return errno;
	}
	else
	{
		return optval;
	}
}

