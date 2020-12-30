#include "HttpResponse.h"
#include "Buffer.h"

void HttpResponse::sendBuffer(Buffer* buf) const
{
	char res[32];
	snprintf(res, sizeof res, "HTTP/1.1 %d ", statusCode_);
	buf->append(res);
	buf->append(statusMessage_);
	buf->append("\r\n");
	
	if(closeConnection_)
		buf->append("Connection: close\r\n");
	else
		buf->append("Connection: Keep-Alive\r\n");
	
	snprintf(res, sizeof res, "Content-Length: %zd\r\n", body_.size());
	buf->append(res);
	
	for(const auto& header : headers_)
	{
		buf->append(header.first);
		buf->append(": ");
		buf->append(header.second);
		buf->append("\r\n");
	}

	buf->append("\r\n");
	buf->append(body_);
}