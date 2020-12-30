#pragma once

#include "InetAddress.h"
#include "Buffer.h"
#include <string>
#include <memory>
#include <functional>

class EventLoop;
class Socket;
class Handle;
class Connection;

typedef std::function<void()> ConnectionCallback;
typedef std::function<void(Buffer*)> ReadCallback;
typedef std::function<void()> WriteCallback;
typedef std::function<void()> CloseCallback;

class Connection
{
public:
	Connection(int id, EventLoop* loop, int connfd, const InetAddress& localAddr, const InetAddress& peerAddr);
	~Connection();
	
	bool isConnected() const { return state_ == connected; }
	
	int id() const { return id_; }
	
	EventLoop* getLoop() { return loop_; }
	
	void setConnectionCallback(const ConnectionCallback& cb)
	{
		connectionCallback_ = cb;
	}
	
	void setReadCallback(const ReadCallback& cb)
	{
		readCallback_ = cb;
	}
	
	void setWriteCallback(const WriteCallback& cb)
	{
		writeCallback_ = cb;
	}
	
	void setCloseCallback(const CloseCallback& cb)
	{
		closeCallback_ = cb;
	}
	
	void connectEstablished();
	void connectClosed();
	void send(const std::string& msg);
	void send(Buffer* buf);
	void shutdown();
	
private:
	enum State { connecting, connected, disconnecting, disconnected };
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();
	const char* stateToString() const;
	void sendInLoop(const char* data, size_t len);
	void shutdownInLoop();

	int id_;
	EventLoop* loop_;
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Handle> handle_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;
	State state_;
	ConnectionCallback connectionCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
	CloseCallback closeCallback_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;
};
