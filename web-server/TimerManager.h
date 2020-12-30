#pragma once

#include "Handle.h"
#include "base/Timestamp.h"

#include <map>
#include <set>
//#include <atomic>

class EventLoop;

typedef std::function<void()> TimerCallback;

class Timer
{
public:
	Timer(TimerCallback cb, Timestamp t, double interval)
		: timerCallback_(std::move(cb))
		, expireTime_(t)
		, interval_(interval)
		, repeat_(interval > 0.0 ? true : false)
		, timerId_(++numTimers)
	{
	}
	
	void timerCallback() const
	{
		timerCallback_();
	}
	
	Timestamp expireTime() const { return expireTime_; }
	bool repeat() const { return repeat_; }
	void restart(Timestamp now);
	
	int timerId() const { return timerId_; }

private:
	const TimerCallback timerCallback_;
	Timestamp expireTime_;
	const double interval_;
	const bool repeat_;
	int timerId_;
	
	//static std::atomic<int> numTimers;
	static int numTimers;
};

struct TimerCmp
{
	bool operator() (Timer *a, Timer *b)
	{
		return a->expireTime() < b->expireTime();
	}
};

class TimerQueue
{
public:
	TimerQueue(EventLoop* loop);
	~TimerQueue();
	
	int addTimer(TimerCallback cb, Timestamp when, double interval);
	void removeTimer(int timerId);
	
private:
	typedef std::multiset<Timer*, TimerCmp> TimerList;

	void handleExpiredTimers();
	void insertTimerInQueue(Timer* timer);
	void removeTimerInQueue(int timerId);
	
	EventLoop* loop_;
	const int timerfd_;
	Handle handle_;
	TimerList timers_;
	std::map<int, Timer*> idTimers_;
};