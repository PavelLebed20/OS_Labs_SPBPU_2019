#!/bin/bash

syslog_location="/etc/syslog.conf"
rsyslog_location="/etc/rsyslog.conf"
tempfile_location="/tmp/disk_monitor_temp"
log_location="/var/log/disk_monitor.log"
log_setting="local0.* $log_location"
pid_location="/var/run/disk_monitor.pid"

function change_syslog {
  local location=$( eval "echo \${$1_location}" )
  grep -v 'local0.' "$location" > "$tempfile_location"
  mv "$tempfile_location" "$location"
  echo "$log_setting" >> "$location"
  service "$1" restart
}

rm -f "$log_location"

if [[ -f "$rsyslog_location" ]]
then
  change_syslog rsyslog
else
  if [[ ! -f "$syslog_location" ]]
  then
    touch "$syslog_location"
  fi
  change_syslog syslog
fi

if [[ ! -f "$pid_location" ]]
then
  touch "$pid_location"
fi

chmod 666 "$pid_location"

g++ -std=c++11 -Wall -Werror types.h utils.h utils.cpp inotify.h inotify.cpp config.h config.cpp daemon.cpp -o daemon
