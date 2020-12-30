#pragma once

#include "File.h"
#include "Mutex.h"

#include <memory>
#include <string>
using std::string;

class LogFile
{
public:
	LogFile(const string& basename, int rollsize, bool threadSafe = true, int flushInterval = 3, int checkEveryNlines = 1024);
	~LogFile();

	void append(const char* logline, int len);
	void flush();
	bool rollFile();
	
private:
	static string getLogFileName(const string& basename, time_t *now);
	
	void append_unlock(const char* logline, int len);

	const string basename_;
	const int rollsize_;
	const int flushInterval_;
	const int checkEveryNlines_;
	std::unique_ptr<MutexLock> mutex_;
	
	int countlines_;
	
	std::unique_ptr<File> file_;
	
	time_t startOfPeriod_;
	time_t lastRoll_;
	time_t lastFlush_;
	
	const static int rollPerSeconds_ = 24*60*60;
};