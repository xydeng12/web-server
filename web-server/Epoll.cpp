#include "Epoll.h"
#include "base/Logger.h"

#include <unistd.h>
#include <errno.h>

Epoll::Epoll(EventLoop* loop)
	: loop_(loop)
	, epollfd_(::epoll_create1(EPOLL_CLOEXEC))
	, events_(initEventListSize)
{
	if(epollfd_ < 0)
	{
		LOG_FATAL << "Epoll::Epoll";
	}
}

Epoll::~Epoll()
{
	::close(epollfd_);
}

void Epoll::wait(HandleList *readyHandles, int timeoutMs)
{
	int numRevents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);

	if(numRevents > 0)
	{
		LOG_INFO << numRevents << " events happened";
		fillReadyHandles(numRevents, readyHandles);
		if(static_cast<size_t>(numRevents) == events_.size())
		{
			events_.resize(events_.size() * 2);
		}
	}
	else if(numRevents == 0)
	{
		LOG_INFO << "nothing happened";
	}
	else
	{
		LOG_ERROR << "Epoll::wait " << "errno = " << errno;
	}
}

void Epoll::fillReadyHandles(int numRevents, HandleList *readyHandles)
{
	for(auto itr = events_.begin(); itr != events_.end() && numRevents > 0; ++itr)
	{
		--numRevents;
		Handle* handle = handles_[(itr->data).fd];
		handle->setRevents(itr->events);
		readyHandles->push_back(handle);
	}
}

void Epoll::updateHandle(Handle* handle)
{
	loop_->assertInLoopThread();
	LOG_INFO << "update handle fd = " << handle->fd() << " events = " << handle->events();
	int fd = handle->fd();
	auto itr = handles_.find(fd);
	
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = handle->events() | EPOLLET;
	event.data.fd = fd;
	if(itr == handles_.end())
	{						
		if(::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) < 0)
		{
			LOG_ERROR << "epoll_ctl op = EPOLL_CTL_ADD fd = " << fd;
		}
		else
		{
			handles_.insert(std::make_pair(fd, handle));
		}
	}
	else
	{
		if(::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0)
		{
			LOG_ERROR << "epoll_ctl op = EPOLL_CTL_MOD fd = " << fd;
		}
		else
		{
			handles_[fd] = handle;
		}
	}
}

void Epoll::removeHandle(Handle* handle)
{
	loop_->assertInLoopThread();
	LOG_INFO << "remove handle fd = " << handle->fd();
	int fd = handle->fd();
	if(::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL) < 0)
	{
		LOG_ERROR << "epoll_ctl op = EPOLL_CTL_DEL fd = " << fd;
	}
	handles_.erase(fd);
}
