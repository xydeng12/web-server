#pragma once

#include <string.h>
#include <string>

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer
{
public:
	FixedBuffer() 
	: cur_(data_)
	{
	}
	
	~FixedBuffer()
	{
	}
	
	void append(const char *buf, size_t len)
	{
		if( (end() - cur_) > len )
		{
			memcpy(cur_, buf, len);
			cur_ += len;
		}
	}
	
	const char* data() const { return data_; }
	int length() const { return static_cast<int>(cur_ - data_); }

	char* current() { return cur_; }
	int avail() const { return static_cast<int>(end() - cur_); }
	void add(size_t len) { cur_ += len; }
	void clear() { bzero(data_, sizeof data_); }
	void reset() { cur_ = data_; }
	
private:
	const char *end() const { return data_ + sizeof data_; }

	char data_[SIZE];
	char *cur_;
}; 

class LogStream
{
public:
	typedef FixedBuffer<kSmallBuffer> Buffer;

	LogStream& operator<<(short);
	LogStream& operator<<(unsigned short);
	LogStream& operator<<(int);
	LogStream& operator<<(unsigned int);
	LogStream& operator<<(long);
	LogStream& operator<<(unsigned long);
	LogStream& operator<<(long long);
	LogStream& operator<<(unsigned long long);
	LogStream& operator<<(bool v)
	{
		buffer_.append(v ? "1" : "0", 1);
		return *this;
	}
	LogStream& operator<<(float v)
	{
		*this << static_cast<double>(v);
		return *this;
	}
	LogStream& operator<<(double);
	LogStream& operator<<(char v)
	{
		buffer_.append(&v, 1);
		return *this;
	}
	LogStream& operator<<(const char *s)
	{
		if(s)
		{
			buffer_.append(s, strlen(s));
		}
		else
		{
			buffer_.append("(null)", 6);
		}
		return *this;
	}
	LogStream& operator<<(const std::string &s)
	{
		buffer_.append(s.c_str(), s.size());
		return *this;
	}
	
	const Buffer& buffer() const
	{
		return buffer_;
	}
	
private:
	static const int kMaxNumSize = 32;

	template<typename T>
	void formatInteger(const char* fmt, T v);

	Buffer buffer_;
};
