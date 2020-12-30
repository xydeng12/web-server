#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

#include <functional>
#include <string>
using std::string;

AsyncLogging::AsyncLogging(const string &basename,
						   int rollsize,
						   int flushInterval)
 : basename_(basename)
 , rollsize_(rollsize)
 , flushInterval_(flushInterval)
 , running_(false)
 , thread_(std::bind(&AsyncLogging::threadFunc, this))
 , mutex_()
 , cond_(mutex_)
 , latch_(1)
 , currentBuffer_(new Buffer)
 , nextBuffer_(new Buffer)
 , buffers_()
{
	currentBuffer_->clear();
	nextBuffer_->clear();
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
	MutexLockGuard lock(mutex_);
	if( currentBuffer_->avail() > len )
	{
		currentBuffer_->append(logline, len);
	}
	else
	{
		buffers_.push_back(std::move(currentBuffer_));
		if(nextBuffer_)
		{
			currentBuffer_ = std::move(nextBuffer_);
		}
		else
		{
			currentBuffer_.reset(new Buffer);
		}
		currentBuffer_->append(logline, len);
		cond_.notify();
	}
}

void AsyncLogging::threadFunc()
{
	assert(running_);
	latch_.countDown();
	LogFile output(basename_, rollsize_);
	BufferPtr buffer1(new Buffer);
	BufferPtr buffer2(new Buffer);
	BufferVector buffersToWrite;
	buffer1->clear();
	buffer2->clear();
	buffersToWrite.reserve(16);
	while(running_)
	{
		{
			MutexLockGuard lock(mutex_);
			if(buffers_.empty())
			{
				cond_.waitForSeconds(flushInterval_);
			}
			buffers_.push_back(std::move(currentBuffer_));
			currentBuffer_ = std::move(buffer1);
			buffersToWrite.swap(buffers_);
			if(!nextBuffer_)
			{
				nextBuffer_ = std::move(buffer2);	
			}
		}
		
		assert(!buffersToWrite.empty());
			
		if(buffersToWrite.size() > 25)
		{
			char buf[256];
			snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
					 Timestamp::now().toString().c_str(), buffersToWrite.size()-2);
			fputs(buf, stderr);
			output.append(buf, static_cast<int>(strlen(buf)));
			buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
		}
			
		for(const auto &buf : buffersToWrite)
		{
			output.append(buf->data(), buf->length());
		}
			
	    if(buffersToWrite.size() > 2)
		{
			buffersToWrite.resize(2);	
		}
		
		if(!buffer1)
		{
			buffer1 = std::move(buffersToWrite[0]);
			buffer1->reset();
		}
		
		if(!buffer2)
		{
			buffer2 = std::move(buffersToWrite[1]);	
			buffer2->reset();
		}
		
		buffersToWrite.clear();
		output.flush();
	}
	output.flush();
}
