#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include <assert.h>

void EventLoopThreadPool::start()
{
	assert(!started_);
	mainLoop_->assertInLoopThread();
	started_ = true;
	for(int i = 0; i < threadsNum_; ++i)
	{
		EventLoopThread* t = new EventLoopThread();
		threads_.push_back(t);
		loops_.push_back(t->startLoop());
	}
}

EventLoop* EventLoopThreadPool::getLoop()
{
	mainLoop_->assertInLoopThread();
	assert(started_);
	EventLoop* loop = mainLoop_;
	if(!loops_.empty())
	{
		loop = loops_[next_];
		++next_;
		if(next_ >= loops_.size())
			next_ = 0;
	}
	return loop;
}