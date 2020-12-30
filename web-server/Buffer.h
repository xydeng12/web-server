#pragma once

#include <vector>
#include <assert.h>
#include <string>
#include <algorithm>
//#include <sys/types.h>
//#include <stddef.h>

class Buffer
{
public:
	static const size_t InitialSize = 4096;
	static const size_t MaxSize = 65536;
	Buffer()
	:	buffer_(InitialSize)
	,	start_(0)
	,	end_(0)
	{}
	
	ssize_t readFd(int fd, int* err);
	ssize_t writeFd(int fd, int* err);
	
	void append(const std::string& data)
	{
		append(data.c_str(), data.size());
	}
	
	void append(const char* data)
	{
		size_t len = 0;
		while(data[len] != '\0')
			++len;
		append(data, len);
	}
	
	void append(const char* data, size_t len)
	{
		if(len > writableSize())
			buffer_.resize(bufferSize() + len);
		std::copy(data, data + len, begin() + end_);
		end_ += len;
	}
	
	const char* start() { return begin() + start_; }
	
	bool isValid(size_t len)
	{
		return (start_ + len >= start_ && start_ + len < end_);
	}

	void startAt(const char* pos);
	
	
	size_t readableSize() const 
	{
		return end_ - start_;
	}
	
	size_t writableSize() const
	{
		return buffer_.size() - end_;
	}
	
	const char* findCRLF() const
	{
		return findCRLF(begin() + start_);
	}
	
	const char* findCRLF(const char* st) const
	{
		const char* cur = st;
		assert(begin() + start_ <= st);
		assert(st < begin() + end_);
		while((cur + 1) != begin() + end_)
		{
			if(*cur == '\r' && *(cur+1) == '\n')
				return cur;
			++cur;
		}
		return NULL;
	}
	
	void reset()
	{
		if(buffer_.size() > MaxSize)
		{
			std::vector<char> temp(InitialSize);
			buffer_.swap(temp);
			start_ = 0;
			end_ = 0;
		}
	}

	void print();

private:
	char* begin() { return &*buffer_.begin(); }
	const char* begin() const { return &*buffer_.begin(); }
	size_t bufferSize() const { return buffer_.size(); } 

	std::vector<char> buffer_;
	size_t start_;
	size_t end_;
};
