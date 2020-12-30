#pragma once

namespace CurrentThread
{
	extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	
	void cacheTid();	
	const char* tidString();
	int tid();
	bool isMainThread();
}
