#ifndef __FIFO_CONNECTION_H__
#define __FIFO_CONNECTION_H__

#include "connection.hpp"

class FifoConn : public Connection
{
public:
	FifoConn();
	bool Open(int Id, bool Create);
	bool Read(void* Buf, size_t Count);
	bool Write(void* Buf, size_t Count);
	virtual bool Close();
private:
	bool is_opened, is_creator;
	string fifo_name;
	sem_t* semaphore;
};

#endif // __FIFO_CONNECTION_H__

