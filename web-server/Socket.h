#pragma once

class InetAddress;

class Socket
{
public:
	Socket(int sockfd)
		: sockfd_(sockfd)
	{ }
	
	~Socket();
	
	int fd() const { return sockfd_; }
	
	void bind(const InetAddress& addr);
	void listen();
	int accept(InetAddress& addr);
	void shutdownWrite();
	void setNoDelay(bool on);
	void setKeepAlive(bool on);
	void close();
	
private:
	int sockfd_;
	
};
