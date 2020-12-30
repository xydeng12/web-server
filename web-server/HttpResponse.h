#pragma once
#include <map>
#include <string>
#include <stdio.h>
using std::string;

class Buffer;

class HttpResponse
{
public:
	HttpResponse(bool close) : statusCode_(0), closeConnection_(close) {}
	
	bool closeConnection()
	{ return closeConnection_; }
	
	void setStatusCode(int code)
	{ statusCode_ = code; }
	
	void setStatusMessage(const string& message)
	{ statusMessage_ = message; }
	
	void setCloseConnection(bool on)
	{ closeConnection_ = on; }
	
	void setContentType(const string& contentType)
	{ addHeader("Content-Type", contentType); }
	
	void addHeader(const string& key, const string& value)
	{ headers_[key] = value; }
	
	void setBody(const string& body)
	{ body_ = body; }
	
	void sendBuffer(Buffer* buf) const;
	
private:
	int statusCode_;
	bool closeConnection_;
	string statusMessage_;
	std::map<string, string> headers_;
	string body_;
};
