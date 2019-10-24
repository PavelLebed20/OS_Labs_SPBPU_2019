#ifndef __PID_H__
#define __PID_H__

#include "defines.hpp"
#include "logger.hpp"


class Pid
{
private:
	string file_name;
	int pid;
public:
	Pid(const string& FileName) : file_name(FileName), pid(-1) {}
	Pid() = default;

	bool Update(int Pid, const Logger &logger)
	{
		pid = Pid;


		ofstream pid_file(file_name);
		if (!pid_file.is_open())
		{
			g_logger.LogError("Cannot open file: " + file_name);
			return false;
		}

		pid_file << pid << std::endl;

		pid_file.close();

		return true;
	}

	bool KillOthers()
	{
		ifstream pid_file(file_name);
		if (!pid_file.is_open())
			return true;
		
		int other_pid;
		while (pid_file >> other_pid)
			kill(other_pid, SIGTERM);

		pid_file.close();

		return true;
	}

	bool Destroy()
	{
		unlink(file_name.c_str());
		return true;
	}

	~Pid()
	{
		Destroy();
	}
};

#endif // __PID_H__

