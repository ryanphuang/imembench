#!/bin/bash

TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4
TACHYON_BIN=$TACHYON_HOME/bin

if [ $# -ne 1 ]; then
  echo "Option is start|stop"
  exit 1
fi

if [ $1 == "start" ]; then
  #0. To be safe (stop another time)
  ./jmxmonitor.sh stop_mm
  $TACHYON_BIN/tachyon-stop.sh
  #1. Format the tachyon cluster
  $TACHYON_BIN/tachyon format
  #2. Start the tachyon cluster
  $TACHYON_BIN/tachyon-start.sh all SudoMount
  #3. Start the monitors
  ./jmxmonitor.sh start_mm
elif [ $1 == "stop" ]; then
  #4. Stop the tachyon cluster
  $TACHYON_BIN/tachyon-stop.sh
  #5. Stop the 
  ./jmxmonitor.sh stop_mm
  #6. Collect the log
  ./jmxmonitor.sh collect_log
  ./gccollector.sh
else
  echo "Option is start|stop"
fi

sleep 0.02
