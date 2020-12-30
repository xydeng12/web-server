#include "Thread.h"
#include "CurrentThread.h"
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

pid_t gettid()
{
	return static_cast<pid_t>(::syscall(SYS_gettid));
}

namespace CurrentThread
{
	__thread int t_cachedTid = 0;
	__thread char t_tidString[32];
	
	void cacheTid()
	{
		if(t_cachedTid == 0)
		{
			t_cachedTid = gettid();
			snprintf(t_tidString, sizeof t_tidString, "%5d", t_cachedTid);
		}
	}
	
	const char* tidString()
	{
		return t_tidString;
	}
	
	int tid()
	{
		if(t_cachedTid == 0)
		{
			cacheTid();	
		}
		return t_cachedTid;
	}
	
	bool isMainThread()
	{
		return tid() == ::getpid();
	}
}

struct ThreadData
{
	typedef Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	pid_t* tid_;
	CountDownLatch* latch_;
	
	ThreadData(ThreadFunc func,
			   pid_t* tid,
			   CountDownLatch* latch)
	 : func_(std::move(func))
	 , tid_(tid)
	 , latch_(latch)
	{
	}
	
	void runInThread()
	{
		*tid_ = CurrentThread::tid();
		tid_ = NULL;
		latch_->countDown();
		latch_ = NULL;
		
		try 
		{
			func_();
		}
		catch(...)
		{
			fprintf(stderr, "exception caught in Thread\n");
			abort();
		}
	}
};

void* startThread(void* obj)
{
	ThreadData* data = static_cast<ThreadData*>(obj);
	data->runInThread();
	delete data;
	return NULL;
}

Thread::Thread(ThreadFunc func)
 : func_(func)
 , tid_(0)
 , pthreadId_(0)
 , started_(false)
 , joined_(false)
 , latch_(1)
{

}

Thread::~Thread()
{
	if(started_ && !joined_)
	{
		pthread_detach(pthreadId_);
	}
}

void Thread::start()
{
	assert(!started_);
	started_ = true;
	ThreadData *data = new ThreadData(func_, &tid_, &latch_);
	if(pthread_create(&pthreadId_, NULL, &startThread, data) != 0)
	{
		started_ = false;
		delete data;
	}
	else
	{
		latch_.wait();
		assert(tid_ > 0);
	}
}

int Thread::join()
{
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(pthreadId_, NULL);
}