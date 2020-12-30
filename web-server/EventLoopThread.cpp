#include "EventLoopThread.h"

EventLoopThread::EventLoopThread()
	: thread_(std::bind(&EventLoopThread::threadFunc, this))
	, eventLoop_(NULL)
	, mutex_()
	, cond_(mutex_)
{
}

EventLoopThread::~EventLoopThread()
{
	if(eventLoop_ != NULL)
	{
		eventLoop_->quit();
		thread_.join();
	}
}

EventLoop* EventLoopThread::startLoop()
{
	assert(!thread_.started());
	thread_.start();
	
	EventLoop* loop = NULL;
	{
		MutexLockGuard lock(mutex_);
		while(eventLoop_ == NULL)
		{
			cond_.wait();
		}
		loop = eventLoop_;
	}
	
	return loop;
}

void EventLoopThread::threadFunc()
{
	EventLoop loop;
	
	{
		MutexLockGuard lock(mutex_);
		eventLoop_ = &loop;
		cond_.notify();
	}
	
	loop.loop();
	
	MutexLockGuard lock(mutex_);
	eventLoop_ = NULL;
}
