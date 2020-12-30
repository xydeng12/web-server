#pragma once

#include "LogStream.h"
#include "Timestamp.h"

class Logger
{
public:
	enum LogLevel
	{
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS
	};
	
	Logger(const char* file, int line);
	Logger(const char* file, int line, LogLevel level);
	Logger(const char* file, int line, LogLevel level, const char* func);
	~Logger();
	
	LogStream& stream() { return impl_.stream_; }
	
	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);
	
	typedef void (*OutputFunc)(const char *msg, int len);
	typedef void (*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);
	
private:
	class Impl
	{
	public:
		Impl(const char* file, int line, LogLevel level);
		void time();
		void filename(const char*);
		void finish();
	
		Timestamp time_;
		LogStream stream_;
		LogLevel level_;
		int line_;
		const char* filename_;
	};
	
	Impl impl_;
};

extern Logger::LogLevel sLogLevel;

inline Logger::LogLevel Logger::logLevel()
{
	return sLogLevel;
}

#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()