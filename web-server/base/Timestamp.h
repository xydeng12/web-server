#pragma once

#include <stdint.h>
#include <string>
using std::string;

class Timestamp
{
public:
	Timestamp()
	: microSecondsSinceEpoch_(0)
	{
	}
	
	Timestamp(int64_t ms)
	: microSecondsSinceEpoch_(ms)
	{
	}
	
	int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
	
	static Timestamp now();
	string toString() const;
	
	static const int microSecondsPerSecond = 1000 * 1000;
	
private:
	int64_t microSecondsSinceEpoch_;
};

inline bool operator < (Timestamp a, Timestamp b)
{
	return a.microSecondsSinceEpoch() < b.microSecondsSinceEpoch();
}

inline bool operator > (Timestamp a, Timestamp b)
{
	return a.microSecondsSinceEpoch() > b.microSecondsSinceEpoch();
}

inline bool operator == (Timestamp a, Timestamp b)
{
	return a.microSecondsSinceEpoch() == b.microSecondsSinceEpoch();
}

inline bool operator <= (Timestamp a, Timestamp b)
{
	return a.microSecondsSinceEpoch() <= b.microSecondsSinceEpoch();
}

inline double timeDifference(Timestamp high, Timestamp low)
{
	int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
	return static_cast<double>(diff) / Timestamp::microSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
	int64_t s = static_cast<int64_t>(seconds * Timestamp::microSecondsPerSecond);
	return Timestamp(timestamp.microSecondsSinceEpoch() + s);
}
