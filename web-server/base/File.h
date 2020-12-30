#pragma once

#include <stdio.h>
#include <string>

class File
{
public:
	File(std::string& filename);
	~File();
	
	int writtenBytes() const { return writtenBytes_; }
	
	void append(const char* msg, size_t len);
	void flush();
private:
	size_t write(const char* msg, size_t len);

	FILE* fp_;
	char buf_[64*1024];
	int writtenBytes_;
};
