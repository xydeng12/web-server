#pragma once

#include <string>
#include <map>
#include <algorithm>
using std::string;

class Uri
{
public:
	Uri() : host_(), path_(), query_() {}

	void setHost(const string& host)
	{ host_ = host; }
	
	string host() const
	{ return host_; }
	
	void setPath(const string& path)
	{ path_ = path; }
	
	string path() const
	{ return path_; }
	
	void setQuery(const string& queryLine)
	{ 
		
	}
	
	bool isResourceFormat(const string& format)
	{
		if(path_.find(format) != string::npos)
			return true;
		else
			return false;
	}
	
private:
	string host_;
	string path_;
	std::map<string, string> query_;
};

class HttpRequest
{	
public:
	enum Method
	{
		INVALID, GET, POST
	};
	enum Version
	{
		UNKNOWN, HTTP10, HTTP11
	};
	
	HttpRequest() : method_(INVALID), version_(UNKNOWN), uri_(), headers_(), bodyLength_(0), body_(string()) {}
	
	void reset() 
	{ 
		method_ = INVALID;
		version_ = UNKNOWN;
		uri_ = Uri();
		headers_.clear();
		bodyLength_ = 0;
		body_ = string();
	}
	
	void setVersion(Version v) { version_ = v; }
	
	Version version() const { return version_; }
	
	string printVersion()
	{
		if(version_ == HTTP10)
			return "HTTP/1.0";
		else if(version_ == HTTP11)
			return "HTTP/1.1";
	}
	
	bool setMethod(const char* start, const char* end)
	{
		string m(start, end);
		if(m == "GET")
			method_ = GET;
		else if(m == "POST")
			method_ = POST;
		else
		{
			method_ = INVALID;
			return false;
		}
		return true;
	}
	
	Method method() const { return method_; }
	
	string printMethod()
	{
		if(method_ == GET)
			return "GET";
		else if(method_ == POST)
			return "POST";
	}
	
	void setUri(const char* start, const char* end)
	{
		const char* question = std::find(start, end, '?');
		if(question != end)
		{
			uri_.setPath(string(start, question - start));
			uri_.setQuery(string(question, end - question));
		}
		else
		{
			uri_.setPath(string(start, end - start));
		}
	}
	
	Uri uri() const
	{
		return uri_;
	}
	
	void setBody(const char* start, const char* end)
	{
		body_.assign(start, end);
	}
	
	int bodyLength() const { return bodyLength_; }
	
	const string& body() const { return body_; }
	
	void addHeader(const char* start, const char* colon, const char* end)
	{
		string field(start, colon);
		++colon;
		while(colon < end && isspace(*colon))
			++colon;
		string value(colon, end);
		if(!value.empty())
			headers_[field] = value; 
	}
	
	string getHeader(const string& field) const
	{
		string result;
		std::map<string, string>::const_iterator it = headers_.find(field);
		if (it != headers_.end())
		{
			result = it->second;
		}
		return result;
	}
	
	void handleHeaders()
	{
		string field = "Host";
		if(headers_.find(field) != headers_.end())
			uri_.setHost(headers_[field]);
		field = "Content-Length";
		if(headers_.find(field) != headers_.end())
			bodyLength_ = std::stoi(headers_[field]);
	}
	
	const std::map<string, string>& headers() const { return headers_; }
	
private:
	Method method_;
	Uri uri_;
	Version version_;
	std::map<string, string> headers_;
	int bodyLength_;
	string body_;
};