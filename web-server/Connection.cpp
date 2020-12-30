#include "Connection.h"
#include "EventLoop.h"
#include "Handle.h"
#include "Socket.h"
#include "SocketUtil.h"

#include "base/Logger.h"

#include <unistd.h>

Connection::Connection(int id, EventLoop* loop, int connfd, const InetAddress& localAddr, const InetAddress& peerAddr)
	: id_(id)
	, loop_(loop)
	, socket_(new Socket(connfd))
	, handle_(new Handle(loop_, connfd))
	, localAddr_(localAddr)
	, peerAddr_(peerAddr)
	, state_(connecting)
{
	handle_->setReadHandler(std::bind(&Connection::handleRead, this));
	handle_->setWriteHandler(std::bind(&Connection::handleWrite, this));
	handle_->setCloseHandler(std::bind(&Connection::handleClose, this));
	handle_->setErrorHandler(std::bind(&Connection::handleError, this));
	socket_->setKeepAlive(true);
	socket_->setNoDelay(true);	
}

Connection::~Connection()
{
	assert(state_ == disconnected);
	
}

void Connection::connectEstablished()
{
	LOG_DEBUG << "Connection::connectEstablished";
	loop_->assertInLoopThread();
	
	assert(state_ == connecting);
	state_ = connected;
	handle_->setReadEvents();
	connectionCallback_();
}

void Connection::connectClosed()
{
	LOG_DEBUG << "Connection::connectClosed";
	loop_->assertInLoopThread();
	
	if(state_ == connected)
	{
		state_ = disconnected;
		handle_->disableAllEvents();
	}
	handle_->remove();
	SocketUtil::close(socket_->fd());
}

void Connection::send(const std::string& msg)
{
	LOG_DEBUG << "Connection::send string";
	if(state_ == connected)
	{
		if(loop_->isInLoopThread())
		{
			sendInLoop(msg.data(), msg.size());
		}
		else
		{
			loop_->runInLoop(std::bind(&Connection::sendInLoop, this, msg.data(), msg.size()));
		}
	}
}

void Connection::send(Buffer* buf)
{
	LOG_DEBUG << "Connection::send buffer";
	if(state_ == connected)
	{
		if(loop_->isInLoopThread())
		{
			sendInLoop(buf->start(), buf->readableSize());
		}
		else
		{
			loop_->runInLoop(std::bind(&Connection::sendInLoop, this, buf->start(), buf->readableSize()));
		}
	}	
}

void Connection::shutdown()
{
	LOG_DEBUG << "Connection::shutdown";
	if(state_ == connected)
	{
		state_ = disconnecting;
		loop_->runInLoop(std::bind(&Connection::shutdownInLoop, this));
	}
}

void Connection::handleRead()
{
	LOG_DEBUG << "Connection::handleRead";
	loop_->assertInLoopThread();

	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(handle_->fd(), &savedErrno);
	if(n > 0)
	{
		readCallback_(&inputBuffer_);
		//inputBuffer_.reset();
	}
	else if(n == 0)
	{
		handleClose();
	}
	else if(savedErrno != 0 && savedErrno != EAGAIN && savedErrno != EWOULDBLOCK)
	{
		errno = savedErrno;
		LOG_ERROR << "Connection::handleRead" << " errno = " << errno;
		handleError();
	}

}

void Connection::handleWrite()
{
	LOG_DEBUG << "Connection::handleWrite";
	loop_->assertInLoopThread();
	if(outputBuffer_.readableSize() == 0)
		return;
	int savedErrno = 0;
	ssize_t n = outputBuffer_.writeFd(handle_->fd(), &savedErrno);
	if(n >= 0)
	{
		if(writeCallback_)
			writeCallback_();
		if(state_ == disconnecting)
			shutdownInLoop();
	}
	else if(savedErrno != 0 && savedErrno != EAGAIN && savedErrno != EWOULDBLOCK)
	{
		errno = savedErrno;
		LOG_ERROR << "Connection::handleWrite" << " errno = " << errno;
		handleError();
	}
}

void Connection::handleClose()
{
	LOG_INFO << "Connection::handleClose " << "fd = " << handle_->fd() << " state = " << stateToString();
	loop_->assertInLoopThread();
	
	if(state_ != connected && state_ != disconnecting)
	{
		LOG_ERROR << "Connection " << id_ << " already disconnected";
		return;
	}
	
	state_ = disconnected;
	handle_->disableAllEvents();
	//socket_->close();
	closeCallback_();
}

void Connection::handleError()
{
	int err = SocketUtil::getSocketError(handle_->fd());
	LOG_ERROR << "Connection::handleError "
			  << "SO_ERROR = " << err;
}

const char* Connection::stateToString() const
{
	switch (state_)
	{
		case disconnected:
			return "disconnected";
    	case connecting:
			return "connecting";
    	case connected:
			return "connected";
		case disconnecting:
			return "disconnecting";
   		default:
			return "unknown state";
  }
}

void Connection::sendInLoop(const char* data, size_t len)
{
	LOG_DEBUG << "Connection::sendInLoop";
	loop_->assertInLoopThread();
	ssize_t writen = 0;
	ssize_t remain = len;
	if(state_ == disconnected)
	{
		LOG_WARN << "disconnected, cannot write";
		return;
	}
	if(outputBuffer_.readableSize() == 0)
	{
		while((writen = SocketUtil::write(handle_->fd(), data + len - remain, remain)) > 0)
		{	
			remain -= writen;
		}
		
		if(remain == 0 && writeCallback_)
		{
			loop_->queueInLoop(std::bind(writeCallback_));
		}
		
		if(writen < 0)
		{
			writen = 0;
			if(errno != EAGAIN && errno != EWOULDBLOCK)
			{
				LOG_ERROR << "Connection::sendInLoop" << " errno = " << errno;
			}
		}
	}

	assert(writen >= 0);
	if(remain > 0)
	{
		size_t oldlen = outputBuffer_.readableSize();
		outputBuffer_.append(data + len - remain, remain);
		if(!handle_->isWriting())
			handle_->setWriteEvents();
	}
}

void Connection::shutdownInLoop()
{
	LOG_INFO << "Connection::shutdownInLoop";
	loop_->assertInLoopThread();
	if(!handle_->isWriting())
		socket_->shutdownWrite();
}
