#ifndef DISK_MONITOR_CONFIG_H
#define DISK_MONITOR_CONFIG_H

#include "types.h"

class config_t
{
public:
    static config_t * get_instance(const char *path=nullptr);
    static void destroy();
    
    void load();
    const set_string_t & get_difference_delete() const;
    const set_string_t & get_difference_add() const;

private: 
    config_t() = delete;
    config_t(config_t const&) = delete;
    config_t& operator=(config_t const&) = delete;

    config_t(string_t &path);
    ~config_t();

    set_string_t load_directories();

    static config_t * _instance;

    string_t _conf_path;
    set_string_t _directories;
    set_string_t _deleted_directories, _added_directories;
};

#endif // DISK_MONITOR_CONFIG_H
