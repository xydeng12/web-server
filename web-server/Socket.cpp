#include "Socket.h"
#include "SocketUtil.h"
#include "InetAddress.h"
#include "base/Logger.h"

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

Socket::~Socket()
{
	SocketUtil::close(sockfd_);
}

void Socket::bind(const InetAddress& addr)
{
	SocketUtil::bind(sockfd_, addr.sockAddr());
}

void Socket::listen()
{
	SocketUtil::listen(sockfd_);
}

int Socket::accept(InetAddress& cliaddr)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	int connfd = SocketUtil::accept(sockfd_, &addr);
	LOG_DEBUG << "Socket::accept fd = " << connfd;
	if(connfd >= 0)
	{
		cliaddr.setSockAddr(addr);
	}
	return connfd;
}

void Socket::shutdownWrite()
{
	SocketUtil::shutdownWrite(sockfd_);
}

void Socket::setNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	int ret = setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
	if(ret < 0)
		LOG_ERROR << "Socket::setNoDelay";
}

void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	int ret = setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
	if(ret < 0)
		LOG_ERROR << "Socket::setKeepAlive";
}

void Socket::close()
{
	SocketUtil::close(sockfd_);	
}