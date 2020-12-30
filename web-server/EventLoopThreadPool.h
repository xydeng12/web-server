#pragma once

#include <vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
	EventLoopThreadPool(EventLoop* mainLoop)
		: mainLoop_(mainLoop)
		, threadsNum_(0)
		, started_(false)
		, next_(0)
	{}
	
	void start();
	
	void setThreadNum(int n)
	{ threadsNum_ = n; }
	
	EventLoop* getLoop();
	
private:
	EventLoop* mainLoop_;
	int threadsNum_;
	bool started_;
	int next_;
	std::vector<EventLoopThread*> threads_;
	std::vector<EventLoop*> loops_;
};