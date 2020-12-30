#pragma once

#include <arpa/inet.h>

namespace SocketUtil
{
	static const int MAXFDS = 100000;
	int createSocket(int family);
	void bind(int sockfd, const struct sockaddr* addr);
	void listen(int sockfd);
	int accept(int sockfd, struct sockaddr_in* addr);
	int connect(int sockfd, const struct sockaddr* servaddr);
	ssize_t read(int sockfd, void *buf, size_t count);
	ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
	ssize_t write(int sockfd, const void *buf, size_t count);
	ssize_t writev(int sockfd, const struct iovec *iov, int iovcnt);
	void close(int sockfd);
	void shutdownWrite(int sockfd);
	
	void IpPortToSockaddr(const char* ip, uint16_t port, struct sockaddr_in* addr);
	void SockaddrToIp(char* buf, size_t size, const struct sockaddr* addr);
	
	struct sockaddr_in getLocalAddress(int sockfd);
	struct sockaddr_in getPeerAddress(int sockfd);
	int getSocketError(int sockfd);
}
