#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#include "utils.h"

string_t home_directory("");

void init_home_directory()
{
    passwd *pw;
    uid_t uid;

    uid = geteuid();
    pw = getpwuid(uid);
    if (pw)
        home_directory = std::string("/home/") + pw->pw_name;
    else
        syslog(LOG_WARNING, "Couldn't find username for UID %u. Paths that beginning by tilde (~) may not work.", uid);
}

string_t get_realpath(std::string &path) 
{
    if (path.length() == 0)
        return path;
    if (path[0] == '~')
        path.replace(0, 1, home_directory); 
    char *canonical_path = realpath(path.c_str(), nullptr);
    if (!canonical_path)
    {
        syslog(LOG_WARNING, "Couldn't find file or directory \"%s\".", path.c_str());
        return std::string("");
    }
    else
    {
        std::string new_path(canonical_path);
        free(canonical_path);
        return new_path;
    }
}

bool is_dir(string_t &path)
{
    struct stat s;
    return !stat(path.c_str(), &s) && S_ISDIR(s.st_mode);
}
