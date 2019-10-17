#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#include "inotify.h"
#include "config.h"
#include "utils.h"

#define PID_FILE "/var/run/disk_monitor.pid"

void exit_daemon(int exit_code=EXIT_SUCCESS) 
{
    closelog();
    inotify_t::destroy();
    config_t::destroy();
    exit(exit_code);
}

void signal_handler(int sig) 
{
    inotify_t * inotify;
    switch (sig) 
    {
    case SIGHUP:
        syslog(LOG_NOTICE, "Hangup signal catched.");
        inotify = inotify_t::get_instance();
        if (inotify)
            inotify->update();
        break;
    case SIGTERM:
        syslog(LOG_NOTICE, "Terminate signal catched. Stopping disk_monitor.");
        exit_daemon(EXIT_SUCCESS);
        break;
    }
}

void terminate_another_instance() 
{
    pid_t prev;
    string_t proc("/proc/");
    ifstream_t pid_file(PID_FILE);

    if (pid_file.is_open() && !pid_file.eof())
    {
        pid_file >> prev;
        if (prev > 0)
        {
            proc += std::to_string(prev);
            kill(prev, SIGTERM); 
        }
    }
    pid_file.close();
}

void save_pid() 
{
    ofstream_t pid_file(PID_FILE);
    if (pid_file.is_open())
    {
        pid_file << getpid();  
        pid_file.close();
    }
}

void make_fork() 
{
    pid_t pid = fork();
    if (pid == -1) 
    {
        syslog(LOG_ERR, "Fork failed. Stopping disk_monitor.");
        exit_daemon(EXIT_FAILURE);
    }
    else if (pid > 0) 
    {
        syslog(LOG_NOTICE, "Successfully made fork. Child's pid is %d.", pid);
        exit_daemon(EXIT_SUCCESS);  
    }
}

void init_config(int argc, char **argv)
{
    config_t * config = nullptr;
    if (argc > 1)
        config = config_t::get_instance(argv[1]);
    if (!config)
    {
        syslog(LOG_ERR, "Couldn't initialize configuration file. Stopping disk_monitor.");
        exit_daemon(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE, "Successfully initialized configuration file.");
}

void init_inotify()
{
    inotify_t * inotify = inotify_t::get_instance();
    if (!inotify)
    {
        syslog(LOG_ERR, "Couldn't initialize inotify. Stopping disk_monitor.");
        exit_daemon(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE, "Successfully initialized inotify.");
}


void init_daemon(int argc, char **argv)
{
    make_fork();

    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Couldn't generate session ID for child process. Stopping disk_monitor.");
        exit_daemon(EXIT_FAILURE);
    }

    make_fork();

    umask(0);

    init_home_directory();
    init_config(argc, argv);  
    init_inotify();

    if ((chdir("/")) < 0)
    {
        syslog(LOG_ERR, "Couldn't change working directory to /. Stopping disk_monitor.");
        exit_daemon(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
}

int main(int argc, char **argv) 
{
    inotify_t * inotify = nullptr;

    try
    {
        openlog("disk_monitor", LOG_NOWAIT | LOG_PID, LOG_LOCAL0);
        syslog(LOG_NOTICE, "Started disk_monitor.");  

        init_daemon(argc, argv);

        terminate_another_instance();
        save_pid();

        inotify = inotify_t::get_instance();
        inotify->update();

        while (true)
            inotify->do_inotify();
    }
    catch (const std::exception &e)
    {
        syslog(LOG_ERR, "%s. Stopping disk_monitor.", e.what());
        exit_daemon(EXIT_FAILURE);
    }
    
    exit_daemon(EXIT_SUCCESS);
}
