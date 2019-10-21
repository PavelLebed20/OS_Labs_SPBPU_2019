//
// Created by chopa on 07.10.2019.
//

#include <iostream>
#include <fstream>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <syslog.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctime>
#include <sys/sendfile.h>
#include <fcntl.h>

#define PID_FILE "/var/run/daemon_lab.pid"
#define TEN_MIN 600

enum modes {
    OLDER = 0,
    YOUNGER = 1
};

std::string cfg_path;
std::string folder1;
std::string folder2;
int interval = 0;
bool need_work = true;

void read_config() {
    std::ifstream cfg_file(cfg_path);
    if (!cfg_file.is_open() || cfg_file.eof()) {
        syslog(LOG_ERR, "Could not open config file or it is empty");
        exit(EXIT_FAILURE);
    }
    cfg_file >> folder1 >> folder2 >> interval;
    cfg_file.close();
}

void kill_prev_daemon() {
    std::ifstream ipidFile(PID_FILE);
    if (ipidFile.is_open() && !ipidFile.eof())
    {
        pid_t prev;
        ipidFile >> prev;
        if (prev > 0) {
            kill(prev, SIGTERM);
        }
    }
    ipidFile.close();
}

void set_pid_file() {
    std::ofstream pid_file(PID_FILE);
    if (!pid_file) {
        syslog(LOG_ERR, "Could not open pid file");
        exit(EXIT_FAILURE);
    }
    pid_file << getpid();
    pid_file.close();
}

void move_file(const char* path_src, const char* path_dst) {
    int source = open(path_src, O_RDONLY, 0);
    int dest = open(path_dst, O_WRONLY | O_CREAT , 0644);

    // struct required, rationale: function stat() exists also
    struct stat stat_source;
    fstat(source, &stat_source);

    sendfile(dest, source, nullptr, stat_source.st_size);

    close(source);
    close(dest);

    if (std::remove(path_src) != 0) {
        syslog(LOG_ERR, "Could not move file");
    }
}

void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGHUP:
            need_work = false;
            read_config();
            syslog(LOG_NOTICE, "Hangup Signal Catched");
            need_work = true;
            break;
        case SIGTERM:
            syslog(LOG_NOTICE, "Terminate Signal Catched");
            unlink(PID_FILE);
            exit(0);
            break;
        default:
            syslog(LOG_NOTICE, "Unknown Signal Catched");
            break;
    }
}


void process_directory(const std::string& folder_src, const std::string& folder_dst, int mode) {
    struct stat t_stat;
    time_t creation_time, cur_time;

    DIR* dir;
    struct dirent *ent;
    dir = opendir(folder_src.c_str());
    if (!dir) {
        syslog(LOG_ERR, "Could not open directory: %s", folder_src.c_str());
        exit(EXIT_FAILURE);
    }
    time ( &cur_time );

    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_type == DT_DIR) {
            continue;
        }
        std::string file_path_src = folder_src + std::string("/") + std::string(ent->d_name);
        std::string file_path_dst = folder_dst + std::string("/") + std::string(ent->d_name);
        stat(file_path_src.c_str(), &t_stat);
        creation_time = t_stat.st_atime;
        switch (mode) {
            case OLDER:
                if ( (cur_time - creation_time) >  TEN_MIN) {
                    move_file(file_path_src.c_str(), file_path_dst.c_str());
                }
                break;
            case YOUNGER:
                if ( (cur_time - creation_time) <  TEN_MIN) {
                    move_file(file_path_src.c_str(), file_path_dst.c_str());
                }
                break;
            default:
                syslog(LOG_NOTICE, "Wrong mode");
        }

    }
    closedir(dir);
}



int main(int argc,char **argv)
{
    pid_t pid = fork();
    if (pid == -1)
        exit(EXIT_FAILURE);
    else if (pid > 0)
        exit(EXIT_SUCCESS);

    if (argc < 2) {
        printf("Wrong numbers of arguments. Expected: 2. Got: %d", argc);
        exit(EXIT_FAILURE);
    }
    cfg_path = argv[1];

    openlog("daemon_lab", LOG_NOWAIT | LOG_PID, LOG_USER);

    umask(0);

    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Could not generate session ID for child process");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1)
        exit(EXIT_FAILURE);
    else if (pid > 0)
        exit(EXIT_SUCCESS);

    read_config();
    cfg_path = realpath(cfg_path.c_str(), nullptr);

    if ((chdir("/")) < 0)
    {
        syslog(LOG_ERR, "Could not change working directory to /");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_NOTICE, "Successfully started daemon_lab");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);

    kill_prev_daemon();
    set_pid_file();

    while (true)
    {
         if(need_work) {
             process_directory(folder1, folder2, OLDER);
             process_directory(folder2, folder1, YOUNGER);
         }
        sleep(interval);
    }
}