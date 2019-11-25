#include "defines.hpp"

#include "host.hpp"

#include "identifier.hpp"

#include "config_reader.hpp"

#include "conn_fifo.hpp"

static const string g_interval_key = "interval";
const string g_pid_file = "/var/run/lebed_demon.pid";

#if  defined(_PID_CONN_) ||  defined(_SOCKET_CONN_)
#else
#define _PID_CONN_
#endif // !_PID_CONN_ | _SOCKET_CONN_



void Host::Init(const string& ConfigFile, const string& PidFile)
{
	Host host = GetInstance();

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


	// Open logg
	g_logger = Logger(g_log_identity);
	// Read config
	string config_file(ConfigFile);
	conf_reader = ConfigReader(config_file);


	if (!conf_reader.Parse())
		exit(EXIT_FAILURE);
	Host::ParseConfig();
	// remember config absolute path
	conf_reader.SetFile(realpath(config_file.c_str(), nullptr));
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
	if (this->pid.KillOthers())
		exit(EXIT_FAILURE);

	// update pid file
	if (!this->pid.Update(getpid(), g_logger))
		exit(EXIT_FAILURE);

	// subscribe for signals
	struct sigaction sig_action;
	memset(&sig_action, 0, sizeof(sig_action));
	sig_action.sa_sigaction = SignalHandler;
	sig_action.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &sig_action, nullptr);
	sigaction(SIGUSR1, &sig_action, nullptr);
	sigaction(SIGHUP, &sig_action, nullptr);
}

void Host::ParseConfig()
{
	Host host = GetInstance();

	if (!host.conf_reader.GetAsInt(g_interval_key, host.interval))
		host.Terminate(EXIT_FAILURE);
	if (host.interval < 1)
	{
		g_logger.LogError("Interval must be positive!");
		host.Terminate(EXIT_FAILURE);
	}
}

void Host::SignalHandler(int Sig, siginfo_t* Info, void *Ptr)
{
	Host host = GetInstance();

	switch (Sig)
	{
	case SIGTERM:
		// Over process
		g_logger.LogNotice("Teminate signal");
		host.Terminate(EXIT_SUCCESS);
		break;
	case SIGHUP:
		host.conf_reader.Parse();
		ParseConfig();
		g_logger.LogNotice("Hangup signal");
		break;
	case SIGUSR1:
		g_logger.LogNotice("User defined 1 signal");
		if (!Host::MakeConnection(Info->si_pid))
			host.Terminate(EXIT_FAILURE);
		break;
	default:
		g_logger.LogNotice("Unknown signal");
		break;
	}

}

bool Host::MakeConnection(int Pid)
{
	Host host = GetInstance();

	if (host.users.find(Pid) != host.users.end())
	{
		g_logger.LogError("User with pid = " + std::to_string(Pid) + " already exists!");
		return true;
	}

	// start connection
#ifdef _PID_CONN_
	Connection* conn = new FifoConn();
#endif // 

	if (!conn->Open(Pid, true))
	{
		delete conn;
		return false;
	}

	return true;
}

void Host::Terminate(int Code)
{
	Host host = GetInstance();

	// kill childs
	for (const auto &t : host.users)
		kill(t.second.pid, SIGTERM);

	// Over process
	host.pid.Destroy();
	exit(Code);
}

void Host::Run()
{
	while (true)
	{
		;
	}
}

Host& Host::GetInstance()
{
	static Host host_runner;
	return host_runner;
}
