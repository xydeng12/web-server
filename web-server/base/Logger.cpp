#include "Logger.h"
#include "CurrentThread.h"

#include <stdlib.h>

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = 
{
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

void defaultOutput(const char *msg, int len)
{
	size_t n = fwrite(msg, 1, len, stdout);
}

void defaultFlush()
{
	fflush(stdout);	
}

__thread Logger::OutputFunc outputFunc = defaultOutput;
__thread Logger::FlushFunc flushFunc = defaultFlush;

Logger::LogLevel initLogLevel()
{
	return Logger::DEBUG;
}

Logger::LogLevel sLogLevel = initLogLevel();

Logger::Impl::Impl(const char* file, int line, LogLevel level)
 : time_(Timestamp::now()), stream_(), level_(level), line_(line)
{
	filename(file);
	time();
	CurrentThread::tid();
	stream_ << CurrentThread::tidString() << " ";
	stream_ << LogLevelName[level];
}

void Logger::Impl::time()
{
	int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::microSecondsPerSecond);
	int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::microSecondsPerSecond);
	struct tm tm_time;
	tm_time = *localtime(&seconds);
	char t_time[64];
	int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d.%06d ",
		tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
		tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
	
	stream_ << t_time;
}

void Logger::Impl::filename(const char *file)
{
	const char* pname = strrchr(file, '/');
	if(pname)
		filename_ = pname + 1;
	else
		filename_ = file;
}

void Logger::Impl::finish()
{
	stream_ << " - " << filename_ << ':' << line_ << '\n';
}

Logger::Logger(const char* file, int line)
 : impl_(file, line, INFO)
{
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
 : impl_(file, line, level)
{
	impl_.stream_ << func << "() ";
}

Logger::Logger(const char* file, int line, LogLevel level)
 : impl_(file, line, level)
{
}

Logger::~Logger()
{
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
	outputFunc(buf.data(), buf.length());
	if(impl_.level_ == FATAL)
	{
		flushFunc();
		abort();
	}
}

void Logger::setLogLevel(Logger::LogLevel level)
{
	sLogLevel = level;
}

void Logger::setOutput(OutputFunc f)
{
	outputFunc = f;
}

void Logger::setFlush(FlushFunc f)
{
	flushFunc = f;	
}
