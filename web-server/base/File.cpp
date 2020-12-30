#include "File.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

File::File(std::string& filename)
 : fp_(::fopen(filename.c_str(), "a"))
 , writtenBytes_(0) 
{
	assert(fp_);
    ::setbuffer(fp_, buf_, sizeof buf_);
}

File::~File()
{
	::fclose(fp_);	
}

void File::append(const char* msg, size_t len)
{
	size_t n = write(msg, len);
	size_t remain = len - n;
	while(remain > 0)
	{
		size_t x = write(msg + n, remain);
		if(x == 0)
		{
			int err = ferror(fp_);
			if(err)
			{
				fprintf(stderr, "File::append() failed %s\n", strerror(err));
			}
			break;
		}
		n += x;
		remain = len - n;
	}
	writtenBytes_ += len;
}

void File::flush()
{
	::fflush(fp_);	
}

size_t File::write(const char* msg, size_t len)
{
	return ::fwrite_unlocked(msg, 1, len, fp_);
}