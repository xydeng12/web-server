#include "Condition.h"

#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

bool Condition::waitForSeconds(double seconds)
{
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	const int64_t kNanoSecondsPerSecond = 1000000000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
	
	time.tv_sec += static_cast<time_t>((time.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
	time.tv_nsec = static_cast<long>((time.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);
	
	mutex_.unassignHolder();
	pthread_cond_timedwait(&cond_, mutex_.getPthreadMutex(), &time);
	mutex_.assignHolder();
}