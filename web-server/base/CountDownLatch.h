#pragma once

#include "Mutex.h"
#include "Condition.h"

class CountDownLatch
{
public:
	CountDownLatch(int count);
	void wait();
	void countDown();
	int getCount();

private:
	int count_;
	MutexLock mutex_;
	Condition cond_;
};