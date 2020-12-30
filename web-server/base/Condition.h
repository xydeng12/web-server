#pragma once

#include "Mutex.h"

#include <pthread.h>

class Condition
{
public:
	Condition(MutexLock& mutex)
	 : mutex_(mutex)
	{
		pthread_cond_init(&cond_, NULL);
	}
	
	~Condition()
	{
		pthread_cond_destroy(&cond_);
	}
	
	void wait()
	{
		mutex_.unassignHolder();
		pthread_cond_wait(&cond_, mutex_.getPthreadMutex());
		mutex_.assignHolder();
	}
	
	bool waitForSeconds(double seconds);
	
	void notify()
	{
		pthread_cond_signal(&cond_);
	}
	
	void notifyAll()
	{
		pthread_cond_broadcast(&cond_);
	}
private:
	pthread_cond_t cond_;
	MutexLock& mutex_;
};