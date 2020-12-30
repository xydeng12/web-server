#pragma once
#include "EventLoop.h"
#include "Handle.h"

#include <vector>
#include <map>
#include <sys/epoll.h>

class Timestamp;

class Epoll
{
public:
	typedef std::vector<Handle*> HandleList;

	Epoll(EventLoop* loop);
	~Epoll();
	
	void wait(HandleList *readyHandles, int timeoutMs);
	void updateHandle(Handle* handle);
	void removeHandle(Handle* handle);
	
	int handles() { return handles_.size(); }
	
private:
	static const int initEventListSize = 16;

	void fillReadyHandles(int numRevents, HandleList *readyHandles_);

	EventLoop* loop_;
	std::map<int, Handle*> handles_;
	
	int epollfd_;
	std::vector<struct epoll_event> events_;
};
