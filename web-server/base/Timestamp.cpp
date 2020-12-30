#include "Timestamp.h"

#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>

Timestamp Timestamp::now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t seconds = tv.tv_sec;
	return Timestamp(seconds * microSecondsPerSecond + tv.tv_usec);
}

string Timestamp::toString() const
{
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / Timestamp::microSecondsPerSecond);
	int microseconds = static_cast<int>(microSecondsSinceEpoch_ % Timestamp::microSecondsPerSecond);
	struct tm tm_time;
	tm_time = *localtime(&seconds);
	char t_time[64];
	snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d.%06d ",
			 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);	
	
	return t_time;
}
