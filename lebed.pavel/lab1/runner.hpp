#pragma once
#include "config_reader.hpp"
#include "pid.hpp"
#include "files_mover.hpp"

class Runner
{
private:
	Runner(const string &pid_file) :
		pid(pid_file) 
	{
	};

public:
	static Runner& GetRunner()
	{
		static Runner g_runner(g_pid_file);

		return g_runner;
	}


	static void ParseConfig()
	{
		int update_interval = 0, old_interval = 0;
		string src, dst;

		if (!Runner::GetRunner().conf_reader.GetAsStr(g_folder1_key, src) ||
			!Runner::GetRunner().conf_reader.GetAsStr(g_folder2_key, dst) ||
			!Runner::GetRunner().conf_reader.GetAsInt(g_update_interval_key, update_interval) ||
			!Runner::GetRunner().conf_reader.GetAsInt(g_old_interval_key, old_interval))
			exit(EXIT_FAILURE);

		if (update_interval <= 0 || old_interval <= 0)
		{
			if (update_interval <= 0)
				g_logger.LogError("Update interval is not positive!");

			if (old_interval <= 0)
				g_logger.LogError("Check file lifetime interval is not positive!");
			exit(EXIT_FAILURE);
		}

		Runner::GetRunner().file_mover = FilesMover(src, dst, (time_t)update_interval, (time_t)old_interval);
	}

	static void SignalHandler(int sig)
	{
		switch (sig)
		{
		case SIGTERM:
			// Over process
			g_logger.LogNotice("Teminate signal");
			Runner::GetRunner().pid.Destroy();
			exit(EXIT_SUCCESS);
			break;
		case SIGHUP:
			Runner::GetRunner().conf_reader.Parse();
			ParseConfig();
			g_logger.LogNotice("Hangup signal");
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
			if (!Runner::GetRunner().file_mover.MoveFiles())
				exit(EXIT_FAILURE);
		}
	}

	ConfigReader conf_reader;
	Pid pid;
	FilesMover file_mover;
};
