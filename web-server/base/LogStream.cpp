#include "LogStream.h"
#include <stdio.h>

LogStream& LogStream::operator<<(short v)
{
	*this << static_cast<int>(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
	*this << static_cast<unsigned int>(v);
	return *this;
}

LogStream& LogStream::operator<<(int v)
{
	formatInteger("%d", v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
	formatInteger("%u", v);
	return *this;
}

LogStream& LogStream::operator<<(long v)
{
	formatInteger("%ld", v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
	formatInteger("%lu", v);
	return *this;
}

LogStream& LogStream::operator<<(long long v)
{
	formatInteger("%lld", v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
	formatInteger("%llu", v);
	return *this;
}

LogStream& LogStream::operator<<(double v)
{
	if( buffer_.avail() >= kMaxNumSize )
	{
		int len = snprintf(buffer_.current(), kMaxNumSize, "%.12g", v);
		buffer_.add(len);
	}
	return *this;
}

template<typename T>
void LogStream::formatInteger(const char* fmt, T v)
{
	if( buffer_.avail() >= kMaxNumSize )
	{
		int len = snprintf(buffer_.current(), kMaxNumSize, fmt, v);
		buffer_.add(len);
	}
}
