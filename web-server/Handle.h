#pragma once

#include <functional>

class EventLoop;
class Timestamp;

class Handle
{
public:
	typedef std::function<void()> EventHandler;

	Handle(EventLoop* loop, int fd);
	~Handle();
	
	int fd() const { return fd_; }
	int events() const { return events_; }
	int revents() const { return revents_; }
	
	void setReadEvents() { events_ |= ReadEvents; update(); }
	void setWriteEvents() { events_ |= WriteEvents; update(); }
	void setRevents(int revents) { revents_ = revents; update(); }
	void disableWrite() { events_ &= ~WriteEvents; update(); }
	void disableAllEvents() { events_ = NoneEvents; update(); }
	bool isReading() const { return events_ & ReadEvents; }
	bool isWriting() const { return events_ & WriteEvents; }
	
	void setReadHandler(EventHandler eh) { readHandler_ = std::move(eh); }
	void setWriteHandler(EventHandler eh) { writeHandler_ = std::move(eh); }
	void setCloseHandler(EventHandler eh) { closeHandler_ = std::move(eh); }
	void setErrorHandler(EventHandler eh) { errorHandler_ = std::move(eh); }
	
	void handleEvent();
	
	void remove();
	
private:
	static const int ReadEvents;
	static const int WriteEvents;
	static const int NoneEvents;
	
	void update();

	EventLoop* loop_;
	int fd_;
	int events_;
	int revents_;
	bool eventHandling_;
	
	EventHandler readHandler_;
	EventHandler writeHandler_;
	EventHandler closeHandler_;
	EventHandler errorHandler_;
};
