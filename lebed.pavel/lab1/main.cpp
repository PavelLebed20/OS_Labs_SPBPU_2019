#include "config_reader.hpp"
#include "logger.hpp"
#include "pid.hpp"
#include "files_mover.hpp"
#include "runner.hpp"

int main(int argc, char** argv)
{
	// Try fork proccess
	pid_t pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			printf("Fork failed!");
		exit(pid == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
	}
	
	// create all files as 0666
	umask(0);

	// Check arguments
	if (argc < 2) 
	{
		printf("Config file does not specified");
		exit(EXIT_FAILURE);
	}

	// Open logg
	g_logger = Logger(g_log_identity);
	// Read config
	string config_file(argv[1]);
	Runner::GetRunner().conf_reader = ConfigReader(config_file);


	if (!Runner::GetRunner().conf_reader.Parse())
		exit(EXIT_FAILURE);
	Runner::ParseConfig();
	// remember config absolute path
	Runner::GetRunner().conf_reader.SetFile(realpath(config_file.c_str(), nullptr));
	// Check child session id
	if (setsid() < 0)
	{
		g_logger.LogError("Failed to generate child process session ID!");
		exit(EXIT_FAILURE);
	}

	pid = fork();
	if (pid != 0)
	{
		if (pid == -1)
			g_logger.LogError("Fork failed!");
		exit(pid == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
	}

	// go to disk root (block unmounting disks)
	if (chdir("/") < 0)
	{
		g_logger.LogError("Failed to set working directory as root (/)");
		exit(EXIT_FAILURE);
	}

	// close input, output and error descriptors (do not need then anymore)
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// deamon successfully created, kill other
	if (!Runner::GetRunner().pid.KillOthers())
		exit(EXIT_FAILURE);


	if (!Runner::GetRunner().pid.Update(getpid(), g_logger))
		exit(EXIT_FAILURE);

	// subscribe for signals
	signal(SIGHUP, Runner::SignalHandler);
	signal(SIGTERM, Runner::SignalHandler);

	// start working
	Runner::Run();
}