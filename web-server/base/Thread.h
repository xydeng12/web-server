#pragma once

#include "CountDownLatch.h"
#include <pthread.h>
#include <functional>

class Thread
{
public:
	typedef std::function<void()> ThreadFunc;
	
	Thread(ThreadFunc);
	~Thread();
	
	void start();
	int join();
	
	bool started() const { return started_; }
	pid_t tid() const { return tid_; }

private:
	pid_t tid_;
	pthread_t pthreadId_;
	ThreadFunc func_;
	bool started_;
	bool joined_;	
	CountDownLatch latch_;
};
