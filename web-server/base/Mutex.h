#pragma once

#include "CurrentThread.h"

#include <pthread.h>
#include <assert.h>

class MutexLock
{
public:
	MutexLock() : holder_(0)
	{
		pthread_mutex_init(&mutex_, NULL);
	}
	~MutexLock()
	{
		assert(holder_ == 0);
		pthread_mutex_destroy(&mutex_);
	}
	
	bool isLockedByThisThread() const
	{
		return holder_ == CurrentThread::tid();
	}
	
	void lock()
	{
		pthread_mutex_lock(&mutex_);
		assignHolder();
	}
	
	void unlock()
	{
		unassignHolder();
		pthread_mutex_unlock(&mutex_);
	}
	
	pthread_mutex_t* getPthreadMutex()
	{
		return &mutex_;
	}
	
private:
	friend class Condition;
	
	void assignHolder()
	{
		holder_ = CurrentThread::tid();
	}

	void unassignHolder()
	{
		holder_ = 0;
	}

	pthread_mutex_t mutex_;
	pid_t holder_;
};

class MutexLockGuard
{
public:
	MutexLockGuard(MutexLock& mutex)
	 : mutex_(mutex)
	{
		mutex_.lock();
	}
	
	~MutexLockGuard()
	{
		mutex_.unlock();
	}
	
private:
	MutexLock& mutex_;
};
