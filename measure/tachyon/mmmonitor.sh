#!/usr/bin/env bash

JMX_HOME="/mnt/tixu/JMXMM223"
HOSTLIST="./peers"

LAUNCHER=
# If debugging is enabled propagate that through to sub-shells
if [[ "$-" == *x* ]]; then
  LAUNCHER="bash -x"
fi

if [ $# -ne 1 ]
then
  echo "please specify the file path to deploy'"
  exit 1
fi

for worker in `cat "$HOSTLIST"|sed  "s/#.*$//;/^$/d"`; do
  echo -n "Connection to $worker as $USER... "
  if [ $1 == "collect_log" ]; then
    scp -r $worker:$JMX_HOME/mmlogs ./tmp
    mv ./tmp/mmlogs/* ./mmlogs/
    rm -Rf ./tmp/mmlogs
  elif [ $1 == "start_mm" ]; then
    ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $JMX_HOME; rm $JMX_HOME/mmlogs/*"
    ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $JMX_HOME; $LAUNCHER $JMX_HOME/startmm.sh worker"
  elif [ $1 == "stop_mm" ]; then
    ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $JMX_HOME; $LAUNCHER $JMX_HOME/stopmm.sh"
  else
    echo "Option is start_mm|stop_mm|collect_log"
  fi
  sleep 0.02
done

wait
