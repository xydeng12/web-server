#include "Buffer.h"
#include "SocketUtil.h"
#include "base/Logger.h"

#include <errno.h>
#include <iostream>

ssize_t Buffer::readFd(int fd, int* err)
{
	reset();
	
	ssize_t n = 0;	
	char extrabuf[MaxSize];
	n = SocketUtil::read(fd, extrabuf, MaxSize);
	if(n < 0)
	{
		*err = errno;
		return -1;	
	}
	if(n <= writableSize())
	{
		std::copy(extrabuf, extrabuf + n, begin() + end_);	
	}
	else
	{
		buffer_.resize(end_ + n);
		std::copy(extrabuf, extrabuf + n, begin() + end_);	
	}
	end_ += n;
	/*	
	while(true)
	{
		char extrabuf[MaxSize];
		n = SocketUtil::read(fd, extrabuf, MaxSize);
		if(n > 0)
		{
			readn += n;		
			end_ += n;	
		}
		else if(n == 0)
		{
			break;	
		}
		else
		{
			if(errno == EAGAIN)
				return readn;
			else
			{
				*err = errno;
				return -1;
			}
		}
	}

	if(errno == EAGAIN || errno == EWOULDBLOCK)
	{
		char extrabuf[65536];
		ssize_t extralen = SocketUtil::read(fd, extrabuf, sizeof extrabuf);
		if(extralen > 0)
		{
			end_ = bufferSize();
			if(static_cast<size_t>(extralen) < start_)
			{
				std::copy(begin() + start_, begin() + end_, begin());
				std::copy(extrabuf, extrabuf + extralen, begin() + end_);
			}
			else
			{
				buffer_.resize(end_ + extralen);
				std::copy(extrabuf, extrabuf + extralen, begin() + end_);
			}
			readn += extralen;
		}
	}
	*/
	LOG_DEBUG << "read " << n << " bytes";
	return n;
}

ssize_t Buffer::writeFd(int fd, int* err)
{
	ssize_t writen = 0;
	ssize_t n = 0;	
	
	while((n = SocketUtil::write(fd, start(), readableSize())) > 0)
	{	
		start_ += n;
		writen += n;
	}
	
	if(n < 0)
	{
		if(errno != EAGAIN && errno != EWOULDBLOCK)
			*err = errno;
	}
	
	if(readableSize() == 0)
	{
		start_ = 0;
		end_ = 0;
	}
	LOG_DEBUG << "write " << writen << " bytes";
	return writen;
}

void Buffer::startAt(const char* pos)
{
	//assert(start() <= pos);
	//assert(pos < begin() + end_);
	size_t len = pos - start();
	start_ += len;
}
	
void Buffer::print()
{
	for(size_t cur = start_; cur < end_; ++cur)
		std::cout << buffer_[cur];
	std::cout << std::endl;
	std::cout << "start_ = " << start_ << std::endl;
	std::cout << "end_ = " << end_ << std::endl;
	std::cout << "bufSize = " << bufferSize() << std::endl;
	std::cout << "readSize = " << readableSize() << std::endl;
	std::cout << "writeSize = " << writableSize() << std::endl;
}
