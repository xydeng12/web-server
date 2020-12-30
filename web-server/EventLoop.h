#pragma once

#include "TimerManager.h"
#include "base/Timestamp.h"
#include "base/Mutex.h"
#include "base/CurrentThread.h"

#include <pthread.h>
#include <memory>
#include <vector>
#include <functional>

class Epoll;
class Handle;

class EventLoop
{
public:
	typedef std::function<void()> Functor;

	EventLoop();
	~EventLoop();
	
	void loop();
	void quit();
	void assertInLoopThread();
	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
	void runInLoop(Functor func);
	void queueInLoop(Functor func);
	void wakeup();
	void updateHandle(Handle* handle);
	void removeHandle(Handle* handle);
	
	int runAt(const Timestamp& time, const TimerCallback& cb);
	int runAfter(double delay, const TimerCallback& cb);
	int runEvery(double interval, const TimerCallback& cb);
	void cancelTimer(int timerId);
	
private:
	typedef std::vector<Handle*> HandleList;
	
	void handleWakeup();
	void doPendingFunctors();

	pid_t threadId_;
	bool looping_;
	bool quit_;
	std::unique_ptr<Epoll> epoll_;
	HandleList readyHandles_;
	Handle* currentReadyHandle_;
	std::unique_ptr<TimerQueue> timerQueue_;
	std::vector<Functor> pendingFunctors_;
	MutexLock mutex_;
	int wakeupFd_;
	bool callingPendingFunctors_;
	Handle* wakeupHandle_;
};
