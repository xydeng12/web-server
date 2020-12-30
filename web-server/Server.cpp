#include "Server.h"
#include "InetAddress.h"
#include "SocketUtil.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "base/Logger.h"
#include <functional>
#include <assert.h>

Server::Server(EventLoop* loop, const InetAddress& listenAddr)
	: loop_(loop)
	, listenSocket_(SocketUtil::createSocket(listenAddr.family()))
	, listenHandle_(loop_, listenSocket_.fd())
	, started_(false)
	, listening_(false)
	, connNum_(0)
	, threadPool_(new EventLoopThreadPool(loop))
{
	listenSocket_.bind(listenAddr);
	listenHandle_.setReadHandler(std::bind(&Server::handleNewConn, this));
}

Server::~Server()
{
	loop_->assertInLoopThread();
	loop_->removeHandle(&listenHandle_);
	
}

void Server::setThreadNum(int num)
{
	assert(num >= 0);
	threadPool_->setThreadNum(num);
}

void Server::start()
{
	if(!started_)
	{
		threadPool_->start();
		assert(!listening_);
		LOG_DEBUG << "Server::start";
		loop_->runInLoop(std::bind(&Server::listen, this));
	}
}

void Server::listen()
{
	loop_->assertInLoopThread();
	LOG_DEBUG << "Server::listen";
	listening_ = true;
	listenSocket_.listen();
	listenHandle_.setReadEvents();
	started_ = true;
}

void Server::handleNewConn()
{
	loop_->assertInLoopThread();
	LOG_DEBUG << "Server::handleNewConn";

	while(true)
	{
		InetAddress cliaddr(0);
		int connfd = listenSocket_.accept(cliaddr);
		if(connfd >= 0)
		{
			if(connfd >= SocketUtil::MAXFDS)
			{
				SocketUtil::close(connfd);
				continue;
			}
			newConnection(connfd, cliaddr);		
		}
		else
		{
			if(errno != EAGAIN)
			{
				LOG_ERROR << "can't accept connection" << " errno = " << errno;
			}
			break;
		}
	}
}

void Server::newConnection(int sockfd, const InetAddress& peerAddr)
{
	loop_->assertInLoopThread();
	EventLoop* ioLoop = threadPool_->getLoop();
	int connId = ++connNum_;
	LOG_INFO << "Server::newConnection" << " id = " << connId;
	InetAddress localAddr(SocketUtil::getLocalAddress(sockfd));
	HttpSessionPtr conn = std::make_shared<HttpSession>(connId, ioLoop, sockfd, localAddr, peerAddr);
	conn->setCloseCallback(std::bind(&Server::removeConnection, this, std::placeholders::_1));
	connections_.insert(std::make_pair(connId, conn));
	ioLoop->runInLoop(std::bind(&HttpSession::sessionEstablished, conn));
}

void Server::removeConnection(const HttpSessionPtr& conn)
{
	LOG_DEBUG << "Server::removeConnection";
	loop_->runInLoop(std::bind(&Server::removeConnectionInLoop, this, conn));	
}

void Server::removeConnectionInLoop(const HttpSessionPtr& conn)
{
	loop_->assertInLoopThread();
	LOG_INFO << "Server::removeConnection " << "connection: " << conn->id(); 
	connections_.erase(conn->id());
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&HttpSession::closeSession, conn));
}
