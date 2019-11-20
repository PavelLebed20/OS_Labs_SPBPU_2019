#ifndef __DEFINES_H__
#define __DEFINES_H__

// using libs
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>

#include "semaphore.hpp"
#include "logger.hpp"

// using types
using string = std::string;
using str_map = std::map<string, string>;
using ifstream = std::ifstream;
using ofstream = std::ofstream;

// global variables
const static string g_pid_file = "/var/run/lebed_demon.pid";
const static string g_log_identity = "lebed_daemon";
const static string g_

#endif // __DEFINES_H__
