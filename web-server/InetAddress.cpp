#include "InetAddress.h"
#include "SocketUtil.h"
#include "base/Logger.h"

#include <string.h>

InetAddress::InetAddress(uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_.sin_port = htons(port);
}

InetAddress::InetAddress(std::string ip, uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	SocketUtil::IpPortToSockaddr(ip.c_str(), port, &addr_);
}

int InetAddress::family() const
{
	return addr_.sin_family;
}

std::string InetAddress::ip() const
{
	char buf[128] = "";
	SocketUtil::SockaddrToIp(buf, sizeof buf, sockAddr());
	return buf;
}

int InetAddress::port() const
{
	return static_cast<int>(addr_.sin_port);
}

const struct sockaddr* InetAddress::sockAddr() const
{
	return reinterpret_cast<const struct sockaddr*>(&addr_);
}

void InetAddress::setSockAddr(const struct sockaddr_in& addr)
{
	LOG_DEBUG << "InetAddress::setSockAddr";
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = addr.sin_family;
	addr_.sin_addr.s_addr = addr.sin_addr.s_addr;
	addr_.sin_port = addr.sin_port;	
}
