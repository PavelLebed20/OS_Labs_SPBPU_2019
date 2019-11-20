#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>



class Logger
{
	using string = std::string;
public:
	Logger(const string &Identity) : identity(Identity), is_opened(false) {}
	Logger() = default;

	void Open()
	{
		if (!is_opened)
		{
			openlog(identity.c_str(), LOG_PID | LOG_NOWAIT, LOG_USER);
			is_opened = true;
		}
			
	}

	void Close()
	{
		if (is_opened)
		{
			closelog();
			is_opened = false;
		}
	}

	void LogError(const string& Msg) const
	{
		syslog(LOG_ERR, "%s", Msg.c_str());
	}

	void LogNotice(const string& Msg) const
	{
		syslog(LOG_NOTICE, "%s", Msg.c_str());
	}

	~Logger()
	{
		Close();
	}

private:
	string identity;
	bool is_opened;
};

static Logger g_logger;

#endif // __LOGGER_H__
