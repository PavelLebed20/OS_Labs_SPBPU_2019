#include "config_reader.hpp"
#include "logger.hpp"
#include "pid.hpp"
#include "files_mover.hpp"

static ConfigReader g_conf_reader;
static Pid g_pid(g_pid_file);
static FilesMover g_file_mover;

static void start_proc()
{
	while (true)
	{
		if (!g_file_mover.MoveFiles())
			exit(EXIT_FAILURE);
	}
}

static void parse_config(string &Folder1, string& Folder2, int &Interval)
{
	Interval = 0;

	if (!g_conf_reader.GetAsStr(g_folder1_key, Folder1) ||
		!g_conf_reader.GetAsStr(g_folder2_key, Folder2) ||
		!g_conf_reader.GetAsInt(g_interval_key, Interval) ||
		!(Interval > 0))
		exit(EXIT_FAILURE);
}

static void signal_handler(int sig)
{
	switch (sig)
	{
	case SIGTERM:
		// Over process
		g_logger.LogNotice("Teminate signal");
		g_pid.Destroy();
		exit(EXIT_SUCCESS);
		break;
	case SIGHUP:
		g_conf_reader.Parse();
		g_logger.LogNotice("Hangup signal");
		break;
	default:
		g_logger.LogNotice("Unknown signal");
		break;
	}
}

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
	g_conf_reader = ConfigReader(config_file);


	if (!g_conf_reader.Parse())
		exit(EXIT_FAILURE);
	string folder1, folder2;
	int interval;
	parse_config(folder1, folder2, interval);
	// remember config absolute path
	g_conf_reader.SetFile(realpath(config_file.c_str(), nullptr));
	// create file mover
	g_file_mover = FilesMover(folder1, folder2, (time_t)interval);

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
	if (!g_pid.KillOthers())
		exit(EXIT_FAILURE);


	if (!g_pid.Update(getpid(), g_logger))
		exit(EXIT_FAILURE);

	// subscribe for signals
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);

	// start working
	start_proc();
}