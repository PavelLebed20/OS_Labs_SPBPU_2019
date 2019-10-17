#include <syslog.h>

#include <algorithm>

#include "config.h"
#include "utils.h"

config_t * config_t::_instance = nullptr;

config_t::config_t(string_t &path) : _conf_path(path)
{};

config_t::~config_t()
{
    _directories.clear();
    _deleted_directories.clear();
    _added_directories.clear();
}

config_t * config_t::get_instance(const char *path)
{
    if (!_instance && path)
    {
        string_t conf_path(path);
        conf_path = get_realpath(conf_path);
        if (conf_path.length() == 0 || is_dir(conf_path))
        {
            syslog(LOG_ERR, "Incorrect path to configuration file.");
            return nullptr;
        }
        _instance = new (std::nothrow) config_t(conf_path);
    }
    return _instance;
}

void config_t::destroy()
{
    delete _instance;
}

set_string_t config_t::load_directories()
{
    ifstream_t config(_conf_path);
    set_string_t directories;
    string_t line, canonical_path;
    if (config.is_open())
    {
        while (std::getline(config, line))
        {
            canonical_path = get_realpath(line);
            if (is_dir(canonical_path))
            {
                syslog(LOG_NOTICE, "Readed directory \"%s\".", line.c_str()); 
                directories.insert(canonical_path);
            }
            else
               syslog(LOG_WARNING, "\"%s\" is not a directory.", line.c_str()); 
        }
        config.close();
    }
    return directories;
}

void config_t::load()
{
    set_string_t directories = load_directories();
    vector_string_t deleted_derictories(_directories.size());
    vector_string_t added_directories(directories.size());
    vector_string_t::iterator it;

    it = std::set_difference(_directories.begin(), _directories.end(),
        directories.begin(), directories.end(), deleted_derictories.begin());
    _deleted_directories = set_string_t(deleted_derictories.begin(), it);
    for (const string_t &path : _deleted_directories)
        syslog(LOG_NOTICE, "Directory \"%s\" was deleted from configuration file.", path.c_str());
    
    it = std::set_difference(directories.begin(), directories.end(), 
        _directories.begin(), _directories.end(), added_directories.begin());
    _added_directories = set_string_t(added_directories.begin(), it);
    for (const string_t &path : _added_directories)
        syslog(LOG_NOTICE, "Directory \"%s\" was added to configuration file.", path.c_str());

    _directories = directories;
}

const set_string_t & config_t::get_difference_delete() const
{
    return _deleted_directories;
}

const set_string_t & config_t::get_difference_add() const
{
    return _added_directories;
}
