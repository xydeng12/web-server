#include "Handle.h"
#include "EventLoop.h"
#include "base/Logger.h"

#include <assert.h>
#include <sys/epoll.h>

const int Handle::ReadEvents =  EPOLLIN | EPOLLPRI;
const int Handle::WriteEvents =  EPOLLOUT;
const int Handle::NoneEvents =  0;

Handle::Handle(EventLoop* loop, int fd)
	: loop_(loop)
	, fd_(fd) 
	, events_(0)
	, revents_(0)
	, eventHandling_(false)
{

}

Handle::~Handle()
{
	assert(!eventHandling_);
}

void Handle::handleEvent()
{
	eventHandling_ = true;
	if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
	{
		LOG_WARN << "fd = " << fd_ << " Handle::handleEvent() POLLHUP";
		if(closeHandler_) closeHandler_();
	}	
	if(revents_ & EPOLLERR)
	{
		if(errorHandler_) errorHandler_();
	}
	
	if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if(readHandler_) readHandler_();
	}
	
	if(revents_ & EPOLLOUT)
	{
		if(writeHandler_) writeHandler_();
	}
	eventHandling_ = false;
}

void Handle::remove()
{
	loop_->removeHandle(this);
}

void Handle::update()
{
	loop_->updateHandle(this);
}
