#!/bin/bash

pid_location="/var/run/lebed_demon.pid"
if [[ ! -f "$pid_location" ]]
then
  touch "$pid_location"
fi

chmod 666 "$pid_location"

g++ -std=c++11 -Wall -Werror config_reader.hpp defines.hpp files_mover.hpp logger.hpp pid.hpp main.cpp -o daemon