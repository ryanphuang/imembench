#!/bin/bash

TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4
TACHYON_BIN=$TACHYON_HOME/bin

if [ $# -ne 1 ]; then
  echo "Option is start-{clean,mm,inst}|stop-{clean,mm,inst}"
  exit 1
fi

if [ $1 == "start-mm" ]; then
  #0. To be safe (stop another time)
  ./jmxmonitor.sh stop_mm
  $TACHYON_BIN/tachyon-stop.sh
  #1. Format the tachyon cluster
  $TACHYON_BIN/tachyon format
  #2. Start the tachyon cluster
  $TACHYON_BIN/tachyon-start.sh all SudoMount
  #3. Start the monitors
  ./jmxmonitor.sh start_mm
elif [ $1 == "stop-mm" ]; then
  #4. Stop the tachyon cluster
  $TACHYON_BIN/tachyon-stop.sh
  #5. Stop the 
  ./jmxmonitor.sh stop_mm
  #6. Collect the log
  ./jmxmonitor.sh collect_log
  ./gccollector.sh
  TIME=`date +"%m-%d-%y-%k-%M-%S"`
  tar zcvf gclogs-$TIME.tar.gz gclogs
  tar zcvf mmlogs-$TIME.tar.gz mmlogs
  rm gclogs/*
  rm mmlogs/*
elif [ $1 == "start-clean" ]; then
  $TACHYON_BIN/tachyon-stop.sh
  $TACHYON_BIN/tachyon format
  $TACHYON_BIN/tachyon-start.sh all SudoMount
elif [ $1 == "stop-clean" ]; then
  $TACHYON_BIN/tachyon-stop.sh
elif [ $1 == "start-inst" ]; then
  $TACHYON_BIN/tachyon-stop.sh
  $TACHYON_BIN/tachyon format
  $TACHYON_BIN/tachyon-start.sh all SudoMount
elif [ $1 == "stop-inst" ]; then
  $TACHYON_BIN/tachyon-stop.sh
  ./alloccollector.sh
  TIME=`date +"%m-%d-%y-%k-%M-%S"`
  tar zcvf alloclogs-$TIME.tar.gz alloclogs
  rm alloclogs/*

else
  echo "Option is start-{clean,mm,inst}|stop-{clean,mm,inst}"
fi

sleep 0.02

