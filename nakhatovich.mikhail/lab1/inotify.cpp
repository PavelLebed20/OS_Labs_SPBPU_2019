#include <limits.h>
#include <syslog.h>
#include <unistd.h>

#include <algorithm>

#include "config.h"
#include "inotify.h"

#define MAX_EVENTS          64 
#define LEN_NAME            (NAME_MAX + 1)
#define EVENT_SIZE          (sizeof(inotify_event)) 
#define BUF_LEN             (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))

inotify_t * inotify_t::_instance = nullptr;

inotify_t::inotify_t(int fd) : _fd(fd)
{};

inotify_t::~inotify_t()
{
    _watch_directories.clear();
}

inotify_t * inotify_t::get_instance()
{
    if (!_instance)
    {
        int fd = inotify_init();
        if (fd >= 0) 
            _instance = new (std::nothrow) inotify_t(fd);
    }
    return _instance;
}

void inotify_t::destroy()
{
    delete _instance;
}

void inotify_t::add_watchers(const set_string_t & directories_to_add)
{
    int wd = -1;

    for (const string_t & path : directories_to_add)
    {
        wd = inotify_add_watch(_fd, path.c_str(), IN_ALL_EVENTS);
        if (wd == -1)
            syslog(LOG_WARNING, "Couldn't add watch to \"%s\".", path.c_str());
        else
        {
            syslog(LOG_NOTICE, "Added watch to \"%s\".", path.c_str());
            _watch_directories.insert(pair_int_string_t(wd, path));
        }
    }
}

void inotify_t::remove_watchers(const set_string_t & directories_to_rm)
{
    map_int_string_t::iterator it;
    
    for (const string_t & path : directories_to_rm)
    {
        auto find_path = [&path](const pair_int_string_t &pair)
        {
            return pair.second == path;
        };

        it = std::find_if(_watch_directories.begin(), _watch_directories.end(), find_path);
        if (it != _watch_directories.end())
        {
            if (inotify_rm_watch(_fd, it->first) == 0)
                syslog(LOG_NOTICE, "Removed watch to \"%s\".", path.c_str());
            _watch_directories.erase(it);
        }
    }
}

void inotify_t::update()
{
    config_t * config = config_t::get_instance();
    if (!config)
        return;
    
    config->load();
    add_watchers(config->get_difference_add());
    remove_watchers(config->get_difference_delete());
}

void log_event(const char *event_name, const char *file_or_dir, const char *filename, const char *directory, const uint32_t *cookie=nullptr)
{
    if (cookie)
        syslog(LOG_NOTICE, "%s: %s \"%s\" in \"%s\". Cookie=%u.", event_name, file_or_dir, filename, directory, *cookie);
    else
        syslog(LOG_NOTICE, "%s: %s \"%s\" in \"%s\".", event_name, file_or_dir, filename, directory);
}

void inotify_t::handle_event(inotify_event *event)
{
    char filename[LEN_NAME] = "/";
    char file_or_dir[10];
    int wd = event->wd;
    uint32_t cookie = event->cookie;
    map_int_string_t::iterator it = _watch_directories.find(wd);
    const char *directory = nullptr;
    if (it != _watch_directories.end())
        directory = it->second.c_str();

    if (event->len)
        sprintf(filename, "%s", (char *)event->name);
    
    if (event->mask & IN_ISDIR)
        sprintf(file_or_dir, "Directory");
    else
        sprintf(file_or_dir, "File");

    switch (event->mask & (IN_ALL_EVENTS | IN_IGNORED))
    {
    case IN_ACCESS:
        log_event("ACCESS", file_or_dir, filename, directory);
        break;
    case IN_MODIFY: 
        log_event("MODIFY", file_or_dir, filename, directory);
        break;
    case IN_ATTRIB: 
        log_event("ATTRIB", file_or_dir, filename, directory);
        break;    
    case IN_CLOSE_WRITE:
        log_event("CLOSE_WRITE", file_or_dir, filename, directory);
        break;     
    case IN_CLOSE_NOWRITE:
        log_event("CLOSE_NOWRITE", file_or_dir, filename, directory);
        break;      
    case IN_OPEN:
        log_event("OPEN", file_or_dir, filename, directory);
        break;      
    case IN_MOVED_FROM:
        log_event("MOVED_FROM", file_or_dir, filename, directory, &cookie);
        break;
    case IN_MOVED_TO:
        log_event("MOVED_TO", file_or_dir, filename, directory, &cookie);
        break;  
    case IN_CREATE:
        log_event("CREATE", file_or_dir, filename, directory);
        break;
    case IN_DELETE: 
        log_event("DELETE", file_or_dir, filename, directory);
        break;
    case IN_DELETE_SELF: 
        log_event("DELETE_SELF", file_or_dir, filename, directory);
        break;
    case IN_MOVE_SELF:
        log_event("MOVE_SELF", file_or_dir, filename, directory);
        break;    
    case IN_IGNORED: 
        syslog(LOG_NOTICE, "IGNORED: \"%s\".", directory);
        break;
    default: 
        syslog(LOG_NOTICE, "UNKNOWN EVENT \"%X\" occured for file \"%s\" in \"%s\".", event->mask, filename, directory);
        break;
    }
}

void inotify_t::do_inotify()
{
    char buffer[BUF_LEN];
    int i = 0, length = read(_fd, buffer, BUF_LEN); 
    inotify_event *event; 
    
    if (length <= 0)
        return;

    while (i < length) 
    {
        event = (inotify_event *)&buffer[i];
        handle_event(event);
        i += EVENT_SIZE + event->len;
    }
}
