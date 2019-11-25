#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "logger.hpp"

class Connection
{
protected:
	size_t mem_size;
public:
	virtual bool Open(int Id, bool Create) { return false; };
	virtual bool Read(void* Buf, size_t Count) { return false; };
	virtual bool Write(void* Buf, size_t Count) { return false; };
	virtual bool Close() { return false; };

	size_t GetSize() const { return mem_size; };
};

#endif // __CONNECTION_H__
