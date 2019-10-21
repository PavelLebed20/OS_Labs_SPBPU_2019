#!/bin/bash

pid_location="/var/run/daemon_lab.pid"

if [[ ! -f "$pid_location" ]]
then
  touch "$pid_location"
fi

chmod 666 "$pid_location"

gcc -Wall -Werror daemon.cpp -o daemon_lab -lstdc++ -std=c++11
