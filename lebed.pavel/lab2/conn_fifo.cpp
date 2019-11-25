#include "defines.hpp"
#include "conn_fifo.hpp"

static const string g_fifo_base_name = "/tmp/lebed_lab2_fifo_";

FifoConn::FifoConn() :
	is_opened(false), is_creator(false)
{
	mem_size = sizeof(*this);
}

bool FifoConn::Open(int Id, bool Create)
{
	if (!Close())
		return false;

	fifo_name = g_fifo_base_name + std::to_string(Id);

	semaphore = sem_open(fifo_name.c_str(), O_CREAT);
	if (semaphore == SEM_FAILED)
	{
		g_logger.LogError("Cannot open semaphore!");
		return false;
	}

	if (Create)
		if (mkfifo(fifo_name.c_str(), 0666) == -1)
		{
			g_logger.LogError("Cannot make '" + fifo_name + "' fifo!");
			sem_unlink(fifo_name.c_str());
			return false;
		}

	is_creator = Create;
	is_opened = true;
	return true;
}

bool FifoConn::Read(void* Buf, size_t Count)
{
	if (!is_opened)
		return false;
	
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		g_logger.LogError("Cannot get current time");
		ts.tv_sec += g_wait_interval;
		return false;
	}

	sem_timedwait(semaphore, &ts);

	int fd = open(fifo_name.c_str(), O_RDONLY);
	if (fd == -1)
	{
		g_logger.LogError("Cannot open '" + fifo_name + "' fifo for reading!");
		return false;
	}
	ssize_t was_read = read(fd, Buf, Count);
	bool result = true;
	if (was_read == -1 or was_read != (ssize_t)Count)
	{
		g_logger.LogError("Cannot read " + std::to_string(Count) + " bytes from '" + fifo_name + "' fifo!");
		result = false;
	}

	close(fd);
	return result;
}

bool FifoConn::Write(void* Buf, size_t Count)
{
	if (!is_opened)
		return false;
	int fd = open(fifo_name.c_str(), O_WRONLY);
	if (fd == -1)
	{
		g_logger.LogError("Cannot open '" + fifo_name + "' fifo for writing!");
		return false;
	}
	bool result = true;
	if (write(fd, Buf, Count) == -1)
	{
		g_logger.LogError("Cannot write " + std::to_string(Count) + " bytes to '" + fifo_name + "' fifo!");
		result = false;
	}

	close(fd);
	return result;
}

bool FifoConn::Close()
{
	if (!is_opened)
		return true;

	bool result = true;
	if (is_creator && remove(fifo_name.c_str()) == -1)
		result = false;

	sem_unlink(fifo_name.c_str());

	is_opened = false;
	return result;
}