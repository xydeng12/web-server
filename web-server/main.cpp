#include "Server.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "base/Logger.h"
#include "base/AsyncLogging.h"
#include <string>
#include <unistd.h>

off_t kRollSize = 500*1000*1000;

AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char *msg, int len)
{
	g_asyncLog->append(msg, len);
}

int main(int argc, char* argv[])
{
	char name[5] = "test";
	AsyncLogging log(::basename(name), kRollSize);
  	log.start();
  	g_asyncLog = &log;
	Logger::setOutput(asyncOutput);
	
  	InetAddress inetaddr(8199);
	int threadsNum = 0;
	if(argc > 1)
		threadsNum = atoi(argv[1]);
	EventLoop loop;
	Server server(&loop, inetaddr);
	server.setThreadNum(threadsNum);
	server.start();
	loop.loop();
}

