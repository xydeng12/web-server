#include "TimerManager.h"
#include "EventLoop.h"
#include "base/Logger.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
extern int errno;
//std::atomic<int> Timer::numTimers;
int Timer::numTimers = 0;

void Timer::restart(Timestamp now)
{
	if(repeat_)
	{
		expireTime_ = addTime(now, interval_);
	}
	else
	{
		expireTime_ = Timestamp();
	}
}

int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
	if(timerfd < 0)
	{
		LOG_FATAL << "timerfd_create failed";
	}
	return timerfd;
}

struct timespec timeFromNow(Timestamp& tm)
{
	int64_t ms = tm.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(ms / Timestamp::microSecondsPerSecond);
	ts.tv_nsec = static_cast<long>((ms % Timestamp::microSecondsPerSecond) * 1000);
	return ts;
}

void setTimerfd(int timerfd, Timestamp expireTime)
{
	struct itimerspec oldValue;
	struct itimerspec newValue;
	memset(&oldValue, 0, sizeof oldValue);
	memset(&newValue, 0, sizeof newValue);
	newValue.it_value = timeFromNow(expireTime);
	LOG_INFO<<"expire time = "<<expireTime.toString();
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if(ret != 0)
	{
		LOG_ERROR << "timerfd_settime()"
				  << " Errno = " << errno;
	}
}

void readTimerfd(int timerfd, Timestamp now)
{
	uint64_t num;
	ssize_t n = ::read(timerfd, &num, sizeof num);
	LOG_INFO << "TimerQueue::handleExpiredTimers() " << num << " at " << now.toString();
	if(n != sizeof num)
	{
		LOG_ERROR << "TimerQueue::handleExpiredTimers() reads " << n << " bytes instead of 8" 
				  << "\nErrno = " << errno;
	}
}

TimerQueue::TimerQueue(EventLoop* loop)
	: loop_(loop)
	, timerfd_(createTimerfd())
	, handle_(loop_, timerfd_)
	, timers_()
	, idTimers_()
{
	handle_.setReadHandler(std::bind(&TimerQueue::handleExpiredTimers, this));
	handle_.setReadEvents();
	loop_->updateHandle(&handle_);
}

TimerQueue::~TimerQueue()
{
	loop_->removeHandle(&handle_);
	::close(timerfd_);
	for(auto it = timers_.begin(); it != timers_.end() ; ++it)
	{
		Timer* tp = *it;
		delete(tp);
	}
	timers_.clear();
	idTimers_.clear();
}

int TimerQueue::addTimer(TimerCallback cb, Timestamp expireTime, double interval)
{
	Timer* timer = new Timer(std::move(cb), expireTime, interval);
	
	loop_->runInLoop(std::bind(&TimerQueue::insertTimerInQueue, this, timer));
	
	return timer->timerId();
}

void TimerQueue::removeTimer(int timerId)
{
	loop_->runInLoop(std::bind(&TimerQueue::removeTimerInQueue, this, timerId));
}

void TimerQueue::handleExpiredTimers()
{
	loop_->assertInLoopThread();
	Timestamp now(Timestamp::now());
	readTimerfd(timerfd_, now);
	
	std::vector<Timer*> expiredTimers;
	while(!timers_.empty() && (*timers_.begin())->expireTime() < now)
	{
		auto top = timers_.begin();
		LOG_INFO << "expire timer id = " << (*top)->timerId();
		expiredTimers.push_back(*top);	
		timers_.erase(top);
		idTimers_.erase((*top)->timerId());
	}
	
	for(auto it = expiredTimers.begin(); it != expiredTimers.end(); ++it)
	{
		(*it)->timerCallback();
	}
	
	for(auto it = expiredTimers.begin(); it != expiredTimers.end(); ++it)
	{
		if((*it)->repeat())
		{
			(*it)->restart(now);
			insertTimerInQueue(*it);
		}
		else
		{
			delete(*it);
		}
	}
	
	if(!timers_.empty())
	{
		setTimerfd(timerfd_, (*timers_.begin())->expireTime());
	}
}

void TimerQueue::insertTimerInQueue(Timer *timer)
{
	if(timers_.empty() || timer->expireTime() < (*timers_.begin())->expireTime())
	{
		setTimerfd(timerfd_, timer->expireTime());
	}	
	timers_.insert(timer);
	idTimers_.insert(std::make_pair(timer->timerId(), timer));
}

void TimerQueue::removeTimerInQueue(int timerId)
{
	auto it = idTimers_.find(timerId);
	if(it != idTimers_.end())
	{
		Timer* tp = it->second;
		timers_.erase(tp);
		delete(tp);
		idTimers_.erase(it);
	}
}
