#pragma once

#include <unistd.h>
#include <string>
using std::string;

namespace ProcessInfo
{
	pid_t pid() { return ::getpid(); }
	
	string hostname();

};

string ProcessInfo::hostname()
{
	char buf[256];
	if(::gethostname(buf, sizeof buf) == 0)
	{
		buf[sizeof(buf)-1] = '\0';
		return buf;
	}
	else
	{
		return "unknownhost";
	}
}
