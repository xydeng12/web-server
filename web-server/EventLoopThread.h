#pragma once

#include "EventLoop.h"
#include "base/Thread.h"
#include "base/Mutex.h"
#include "base/Condition.h"

#include <string>

class EventLoopThread
{
public:
	EventLoopThread();
	~EventLoopThread();
	
	EventLoop* startLoop();
private:
	void threadFunc();

	Thread thread_;
	EventLoop* eventLoop_;
	MutexLock mutex_;
	Condition cond_;
};