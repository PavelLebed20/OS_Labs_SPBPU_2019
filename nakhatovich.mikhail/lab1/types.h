#ifndef DISK_MONITOR_TYPES_H
#define DISK_MONITOR_TYPES_H

#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

typedef std::string               string_t;
typedef std::set<string_t>        set_string_t; 
typedef std::pair<int, string_t>  pair_int_string_t;
typedef std::map<int, string_t>   map_int_string_t;
typedef std::ifstream             ifstream_t;
typedef std::ofstream             ofstream_t;
typedef std::vector<string_t>     vector_string_t;

#endif // DISK_MONITOR_TYPES_H
