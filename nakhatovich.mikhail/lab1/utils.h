#ifndef DISK_MONITOR_UTILS_H
#define DISK_MONITOR_UTILS_H

#include "types.h"

void init_home_directory();
string_t get_realpath(string_t &path);
bool is_dir(string_t &path);

#endif // DISK_MONITOR_UTILS_H
