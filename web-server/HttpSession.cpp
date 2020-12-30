#include "HttpSession.h"
#include "HttpResponse.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "base/Logger.h"
#include "base/Timestamp.h"
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

void defaultHttpHandler(const HttpRequest& request, HttpResponse* response)
{
	if(request.method() == HttpRequest::GET)
	{
		if(request.uri().path() == "/")
		{
			response->setStatusCode(200);
			response->setStatusMessage("OK");
			response->setContentType("text/html");
			response->addHeader("Server", "Muduo");
			string now = Timestamp::now().toString();
			response->setBody("<html><head><title>This is title</title></head>"
						  "<body><h1>Hello</h1>Now is " + now +
						  "</body></html>");
		}
		/*
		std::string root = "./data";
		std::string pathname = root + request.uri().path();
		if(request.uri().path() == "/")
			pathname += "index.html";
		std::ifstream in;
		in.open(pathname.c_str());
		if(!in)
		{
			LOG_ERROR << "Fail to open " + pathname;
			response->setStatusCode(404);
			response->setStatusMessage("Not Found");
			return;
		}
		std::ostringstream os;
		os << in.rdbuf();
		response->setBody(os.str());
		
		response->setStatusCode(200);
		response->setStatusMessage("OK");
		
		if(request.uri().isResourceFormat(".html"))
			response->setContentType("text/html");
		else if(request.uri().isResourceFormat(".jpg"))
			response->setContentType("image/jpeg");
			*/
	}
}

HttpSession::HttpSession(int id,
						 EventLoop* loop, 
						 int connfd, 
						 const InetAddress& localAddr, 
						 const InetAddress& peerAddr)
	: connection_(new Connection(id, loop, connfd, localAddr, peerAddr))
	, timerId_(-1)
	, state_(PARSE_STATE_REQUESTLINE) 
	, httpHandler_(defaultHttpHandler)
{
	connection_->setConnectionCallback(std::bind(&HttpSession::onConnection, this));
	connection_->setReadCallback(std::bind(&HttpSession::onMessage, this, std::placeholders::_1));
	connection_->setCloseCallback(std::bind(&HttpSession::onClose, this));
}

void HttpSession::sessionEstablished()
{
	connection_->connectEstablished();	
}

void HttpSession::closeSession()
{
	connection_->connectClosed();
}

bool HttpSession::parseRequest(Buffer* buf)
{
	bool parsed = true;
	bool parsing = true;
	while(parsing)
	{
		switch(state_)
		{
			case PARSE_STATE_REQUESTLINE:
			{
				const char *crlf = buf->findCRLF();
				if(crlf)
				{
					parsed = parseRequestLine(buf->start(), crlf);
					if(parsed)
					{
						//if(buf->isValid(crlf - buf->start() + 2))
						//{
							buf->startAt(crlf + 2);
							state_ = PARSE_STATE_HEADER;
						//}
					}
					else
						parsing = false;
				}
				else
				{
					parsing = false;
				}
				break;
			}
			case PARSE_STATE_HEADER:
			{
				const char *crlf = buf->findCRLF();
				if(crlf)
				{
					const char* colon = std::find(buf->start(), crlf, ':');
					if(colon != crlf)
						request_.addHeader(buf->start(), colon, crlf);
					else
					{
						request_.handleHeaders();
						if(request_.bodyLength() > 0)
							state_ = PARSE_STATE_BODY;
						else
							state_ = PARSE_STATE_FINISHED;
					}
					//if(buf->isValid(crlf - buf->start() + 2))
						buf->startAt(crlf + 2);
						
				}
				else
				{
					parsing = false;
				}
				break;
			}
			case PARSE_STATE_BODY:
			{/*
				int bodyLength = request_.bodyLength();
				if(bodyLength <= buf->readableSize())
				{
					request_.setBody(buf->start(), buf->start() + bodyLength);
					buf->startAt(buf->start() + bodyLength);
					
				}
				else
				{
					request_.setBody(buf->start(), buf->start() + buf->readableSize());
					buf->reset();
				}
*/
				state_ = PARSE_STATE_FINISHED;
				parsing = false;
				break;
			}
			case PARSE_STATE_FINISHED:
			{
				parsing = false;
				break;
			}
			default:
			{
				parsed = false;
				parsing = false;
			}
		}
	}
	return parsed;
}

bool HttpSession::parseRequestLine(const char* begin, const char* end)
{
	const char* cur = begin;
	const char* space = std::find(cur, end, ' ');
	if(space != end && request_.setMethod(cur, space))
	{
		cur = space + 1;
		space = std::find(cur, end, ' ');
		if(space != end)
		{
			request_.setUri(cur, space);
			cur = space + 1;
			if(end - cur == 8 && std::equal(cur, end-1, "HTTP/1."))
			{
				if(*(end-1) == '0')
				{
					request_.setVersion(HttpRequest::HTTP10);
					return true;
				}
				else if(*(end-1) == '1')
				{
					request_.setVersion(HttpRequest::HTTP11);
					return true;
				}
			}
		}
	}
	return false;
}

void HttpSession::resetRequest()
{
	state_ = PARSE_STATE_REQUESTLINE;
	request_.reset();
}

void HttpSession::handleRequest()
{
	if(state_ != PARSE_STATE_FINISHED)
		return;
	
	const std::string& connection = request_.getHeader("Connection");
	bool close = (connection == "close") || (request_.version() == HttpRequest::HTTP10 && connection != "Keep-Alive");
	HttpResponse response(close);
	httpHandler_(request_, &response);
	Buffer buf;
	response.sendBuffer(&buf);
	connection_->send(&buf);
	if (response.closeConnection())
	{
		connection_->shutdown();
	}
}

void HttpSession::onConnection()
{
	if(connection_->isConnected())
	{
		timerId_ = getLoop()->runAfter(60.0, std::bind(&HttpSession::closeSession, this));
	}

}

void HttpSession::onMessage(Buffer* buf)
{
	getLoop()->cancelTimer(timerId_);
	timerId_ = getLoop()->runAfter(60.0, std::bind(&HttpSession::closeSession, this));
	//buf->print();

	if(!parseRequest(buf))
	{
		LOG_ERROR << "HTTP request parsing error";
		connection_->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		connection_->shutdown();
	}
	
	if(state_ == PARSE_STATE_FINISHED)
	{
		LOG_DEBUG << "HTTP request parsed";
		handleRequest();
		resetRequest();
	}
}

void HttpSession::onClose()
{
	LOG_DEBUG << "HttpSession::onClose";
	sessionClosedCallback_(shared_from_this());	
}
