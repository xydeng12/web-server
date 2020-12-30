#pragma once

#include "LogStream.h"
#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"
#include "CountDownLatch.h"

#include <memory>
#include <vector>
#include <string>
using std::string;

class AsyncLogging
{
public:
	AsyncLogging(const string &basename,
				 int rollsize,
				 int flushInterval = 3);
	~AsyncLogging()
	{
		if(running_)
		{
			stop();
		}
	}
	
	void append(const char* logline, int len);
	
	void start()
	{
		running_ = true;
		thread_.start();
		
	}
	
	void stop()
	{
		running_ = false;
		cond_.notify();
		thread_.join();	
	}
private:
	void threadFunc();
	
	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
	typedef BufferVector::value_type BufferPtr;

	const string basename_;
	const int rollsize_;
	const int flushInterval_;
	bool running_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
	CountDownLatch latch_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
};