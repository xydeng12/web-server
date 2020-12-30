#include "LogFile.h"
#include "ProcessInfo.h"
#include <sys/time.h>
#include <stdio.h>

LogFile::LogFile(const string& basename, int rollsize, bool threadSafe, int flushInterval, int checkEveryNlines)
 : basename_(basename)
 , rollsize_(rollsize)
 , flushInterval_(flushInterval)
 , checkEveryNlines_(checkEveryNlines)
 , countlines_(0)
 , startOfPeriod_(0)
 , lastRoll_(0)
 , lastFlush_(0)
 , mutex_(threadSafe ? new MutexLock : NULL)
{
	rollFile();
}

LogFile::~LogFile()
{
}

void LogFile::append(const char* logline, int len)
{
	if(mutex_)
	{
		MutexLockGuard lock(*mutex_);
		append_unlock(logline, len);
	}
	else 
	{
		append_unlock(logline, len);
	}
}

void LogFile::flush()
{
	if(mutex_)
	{
		MutexLockGuard lock(*mutex_);
		file_->flush();
	}
	else 
	{
		file_->flush();
	}
}

bool LogFile::rollFile()
{
	time_t now = 0;
	string filename = getLogFileName(basename_, &now);
	time_t start = now / rollPerSeconds_ * rollPerSeconds_;	
	if(now > lastRoll_)
	{
		lastRoll_ = now;
		lastFlush_ = now;
		startOfPeriod_ = start;
		file_.reset(new File(filename));
		return true;
	}
	return false;
}

string LogFile::getLogFileName(const string& basename, time_t *now)
{
	string filename;
	filename = basename;
	
	char timebuf[32];
	struct tm tm_time;
	*now = ::time(NULL);
	tm_time = *localtime(now);
	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm_time);
	filename += timebuf;
	
	filename += ProcessInfo::hostname();
	
	char pidbuf[32];
	snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
	filename += pidbuf;
	
	filename += ".log";
	
	return filename;
}

void LogFile::append_unlock(const char* logline, int len)
{
	file_->append(logline, len);
	if(file_->writtenBytes() > rollsize_)
	{
		rollFile();	
	}
	else
	{
		++countlines_;
		if(countlines_ >= checkEveryNlines_)	
		{
			countlines_ = 0;
			time_t now = ::time(NULL);
			time_t thisPeriod_ = now / rollPerSeconds_ * rollPerSeconds_;
			if(thisPeriod_ != startOfPeriod_)
			{
				rollFile();
			}
			else if(now - lastFlush_ > flushInterval_)
			{
				lastFlush_ = now;
				file_->flush();
			}
		}
	}
}