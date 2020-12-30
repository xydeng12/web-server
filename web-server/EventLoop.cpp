#include "EventLoop.h"
#include "Handle.h"
#include "Epoll.h"
#include "base/Logger.h"
#include "base/CurrentThread.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <signal.h>

class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
IgnoreSigPipe initObj;

__thread EventLoop* tLoopInThisThread = NULL;

const int epollTimeMs = 10000;
int createEventfd()
{
	int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if(fd < 0)
	{
		LOG_ERROR << "eventfd failed";	
		abort();
	}
	return fd;
}

EventLoop::EventLoop()
	: threadId_(CurrentThread::tid())
	, looping_(false)
	, quit_(false)
	, epoll_(new Epoll(this))
	, timerQueue_(new TimerQueue(this))
	, pendingFunctors_()
	, mutex_()
	, wakeupFd_(createEventfd())
	, callingPendingFunctors_(false)
	, wakeupHandle_(new Handle(this, wakeupFd_))
	, currentReadyHandle_(NULL)
{
	LOG_INFO << "EventLoop " << this << " created in thread " << threadId_;
	if(tLoopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " << tLoopInThisThread
				  << " exists in this thread " << threadId_;
	}
	else
	{
		tLoopInThisThread = this;
	}
		
	wakeupHandle_->setReadHandler(std::bind(&EventLoop::handleWakeup, this));
	wakeupHandle_->setReadEvents();
	updateHandle(wakeupHandle_);
}

EventLoop::~EventLoop()
{
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << CurrentThread::tid();
	removeHandle(wakeupHandle_);
	delete(wakeupHandle_);
	close(wakeupFd_);
	tLoopInThisThread = NULL;
}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;
	LOG_INFO << "EventLoop " << this << " start looping";
	int i = 1;
	while(!quit_)
	{
		//LOG_INFO << "loop time " << i++;
		readyHandles_.clear();
		epoll_->wait(&readyHandles_, epollTimeMs);
		for(Handle* handle : readyHandles_)
		{
			currentReadyHandle_ = handle;
			currentReadyHandle_->handleEvent();
		}
		currentReadyHandle_ = NULL;
		doPendingFunctors();
	}
	LOG_INFO << "EventLoop " << this << " stop looping";
	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	if(!isInLoopThread())
	{
		wakeup();	
	}
}

void EventLoop::assertInLoopThread()
{
	if(!isInLoopThread())
	{
		LOG_FATAL << "EventLoop::assertInLoopThread - EventLoop " << this
            	  << " was created in threadId_ = " << threadId_
            	  << ", current thread id = " <<  CurrentThread::tid();
	}
}

void EventLoop::runInLoop(Functor func)
{
	if(isInLoopThread())
	{
		func();
	}
	else
	{
		queueInLoop(std::move(func));
	}
}

void EventLoop::queueInLoop(Functor func)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(func));
	}
	
	if(!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = write(wakeupFd_, &one, sizeof one);
	if(n != sizeof one)
	{
		LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";	
	}
}

void EventLoop::handleWakeup()
{
	uint64_t one = 1;
	ssize_t n = read(wakeupFd_, &one, sizeof one);
	if(n != sizeof one)
	{
		LOG_ERROR << "EventLoop::handleWakeup() reads " << n << " bytes instead of 8";	
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for(const Functor& func : functors)
	{
		func();
	}
	callingPendingFunctors_ = false;
}

void EventLoop::updateHandle(Handle* handle)
{
	assertInLoopThread();
	epoll_->updateHandle(handle);
}

void EventLoop::removeHandle(Handle* handle)
{
	assertInLoopThread();
	epoll_->removeHandle(handle);
}

int EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
	return timerQueue_->addTimer(cb, time, 0.0);
}

int EventLoop::runAfter(double delay, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, cb);
}

int EventLoop::runEvery(double interval, const TimerCallback& cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::cancelTimer(int timerId)
{
	timerQueue_->removeTimer(timerId);
}
