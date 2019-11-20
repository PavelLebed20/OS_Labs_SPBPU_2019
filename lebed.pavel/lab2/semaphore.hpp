#ifndef __SEMAPHORE_HPP__
#define __SEMAPHORE_HPP__

#include <mutex>
#include <condition_variable>
#include <chrono>

class Semaphore 
{
public:
	Semaphore(int Count = 0)
		: _count(Count)
	{
	}

	inline void notify()
	{
		std::unique_lock<std::mutex> lock(_mtx);
		_count++;
		_cv.notify_one();
	}

	inline bool wait(time_t Timeout = g_def_timeout)
	{
		std::unique_lock<std::mutex> lock(_mtx);
		
		while (_count == 0)
		{
			if ((_cv.wait_for(lock, std::chrono::seconds(Timeout))) == std::cv_status::timeout)
				return false;
		}
			
		_count--;
		return true;
	}

private:
	std::mutex _mtx;
	std::condition_variable _cv;
	int _count;
public:
	static const time_t g_def_timeout = 5; // seconds
};

#endif // ! __SEMAPHORE_HPP__

// END OD 'Semaphore.hpp' file
