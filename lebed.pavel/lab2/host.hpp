#ifndef __HOST_H__
#define __HOST_H__

#include "config_reader.hpp"
#include "pid.hpp"

class Host
{
private:
	Host(const string &pid_file) :
		pid(pid_file) 
	{
	};

public:
	static Host& GetRunner()
	{
		static Host g_runner(g_pid_file);

		return g_runner;
	}


	static void ParseConfig()
	{

	}

	static void SignalHandler(int sig)
	{
		switch (sig)
		{
		case SIGTERM:
			// Over process
			g_logger.LogNotice("Teminate signal");
			Host::GetRunner().pid.Destroy();
			exit(EXIT_SUCCESS);
			break;
		case SIGHUP:
			Host::GetRunner().conf_reader.Parse();
			ParseConfig();
			g_logger.LogNotice("Hangup signal");
			break;
		case SIGUSR1:
			g_logger.LogNotice("User defined 1 signal");
			break;
		default:
			g_logger.LogNotice("Unknown signal");
			break;
		}
	}

	static void Run()
	{
		while (true)
		{
			;
		}
	}

	ConfigReader conf_reader;
	Pid pid;
};

#endif // __HOST_H__

