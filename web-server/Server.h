#pragma once

#include "Socket.h"
#include "Handle.h"
#include "HttpSession.h"

#include <map>
#include <memory>

class EventLoop;
class EventLoopThreadPool;
class InetAddress;

class Server
{
public:
	Server(EventLoop* loop, const InetAddress& listenAddr);
	~Server();
	
	void setThreadNum(int num);
	void start();
		
private:	
	void listen();
	void handleNewConn();
	void newConnection(int sockfd, const InetAddress& addr);
	void removeConnection(const HttpSessionPtr& conn);
	void removeConnectionInLoop(const HttpSessionPtr& conn);

	EventLoop* loop_;
	Socket listenSocket_;
	Handle listenHandle_;
	bool started_;
	bool listening_;
	int connNum_;
	std::shared_ptr<EventLoopThreadPool> threadPool_;
	std::map<int, HttpSessionPtr> connections_;
};
