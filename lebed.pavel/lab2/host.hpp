#ifndef __HOST_H__
#define __HOST_H__

#include <map>

#include "config_reader.hpp"
#include "pid.hpp"
#include "connection.hpp"

#include <memory>

class Host
{
private:
	Host() = default;

public:
	void Init(const string& ConfigFile, const string& PidFile);

	static Host& GetInstance();

	static void Run();

private:
	struct UserInfo
	{
		int pid;
		Connection* conn;

		UserInfo(int Pid, Connection *Conn) :
			pid(Pid), conn(Conn)
		{}
	};

	static void ParseConfig();

	static void SignalHandler(int Sig, siginfo_t* Info, void* Ptr);

	static void Terminate(int Code);

	static bool MakeConnection(int Pid);

	ConfigReader conf_reader;
	Pid pid;
	std::map<int, UserInfo> users;
	int interval;
};

#endif // __HOST_H__

