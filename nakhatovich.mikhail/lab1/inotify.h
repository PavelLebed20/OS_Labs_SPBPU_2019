#ifndef DISK_MONITOR_INOTIFY_H
#define DISK_MONITOR_INOTIFY_H

#include <sys/inotify.h>

#include "types.h"

class inotify_t
{
public:
    static inotify_t * get_instance();
    static void destroy();

    void do_inotify();
    void update();

private:
    inotify_t() = delete;
    inotify_t(inotify_t const&) = delete;
    inotify_t& operator=(inotify_t const&) = delete;

    inotify_t(int fd);
    ~inotify_t();

    void handle_event(inotify_event *event);
    void add_watchers(const set_string_t & directories_to_add);
    void remove_watchers(const set_string_t & directories_to_rm);

    static inotify_t * _instance;

    int _fd;
    map_int_string_t _watch_directories;
};

#endif // DISK_MONITOR_INOTIFY_H
