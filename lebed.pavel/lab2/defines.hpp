#ifndef __DEFINES_H__
#define __DEFINES_H__

// using libs
#include <string>
#include <map>
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <csignal>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>

#include "semaphore.hpp"
#include "logger.hpp"

// using types
using string = std::string;
using str_map = std::map<string, string>;
using ifstream = std::ifstream;
using ofstream = std::ofstream;

// global variables
const static string g_log_identity = "lebed_daemon";
const static string g_pid_host = "lebed_daemon_host_pid";

static int g_wait_interval = 5;

#endif // __DEFINES_H__
