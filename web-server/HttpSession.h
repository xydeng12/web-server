#pragma once

#include "InetAddress.h"
#include "HttpRequest.h"
#include "Connection.h"
#include <memory>
#include <functional>

class EventLoop;
class Buffer;
class HttpSession;
class HttpResponse;

typedef std::shared_ptr<HttpSession> HttpSessionPtr;
typedef std::function<void(const HttpSessionPtr&)> SessionClosedCallback;
typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession>
{
public:
	enum PARSE_STATE
	{
		PARSE_STATE_REQUESTLINE, PARSE_STATE_HEADER, PARSE_STATE_BODY, PARSE_STATE_FINISHED
	};

	HttpSession(int id, EventLoop* loop, int connfd, const InetAddress& localAddr, const InetAddress& peerAddr);
	
	int id() const { return connection_->id(); }
	
	EventLoop* getLoop() { return connection_->getLoop(); }
	
	void sessionEstablished();
	
	void closeSession();
		
	void setCloseCallback(const SessionClosedCallback& cb)
	{
		sessionClosedCallback_ = cb;
	}
	
	void setHttpHandler(const HttpHandler& h)
	{
		httpHandler_ = h;
	}
	
	bool parseRequest(Buffer* buf);

private:
	bool parseRequestLine(const char* begin, const char* end);
	void resetRequest();
	void handleRequest();
	void onConnection();
	void onMessage(Buffer* buf);
	void onClose();
	void handleExpired();
	
	Connection* connection_;
	SessionClosedCallback sessionClosedCallback_;
	int timerId_;
	
	HttpRequest request_;
	PARSE_STATE state_;
	HttpHandler httpHandler_;
};
