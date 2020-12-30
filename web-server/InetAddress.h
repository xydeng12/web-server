#pragma once

#include <netinet/in.h>
#include <string>

class InetAddress
{
public:
	InetAddress(uint16_t port = 0);
	InetAddress(std::string ip = "", uint16_t port = 0);
	InetAddress(const struct sockaddr_in& addr)
		: addr_(addr)
	{ }
	
	int family() const;
	std::string ip() const;
	int port() const;
	
	const struct sockaddr* sockAddr() const;
	void setSockAddr(const struct sockaddr_in& addr);
	
private:
	struct sockaddr_in addr_;
};
