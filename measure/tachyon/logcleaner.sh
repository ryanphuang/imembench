#!/usr/bin/env bash

TACHYON_HOME="/mnt/tachyon/tachyon-0.6.4"
JMX_HOME="/mnt/tixu/JMXMM223"
HOSTLIST="./peers"

LAUNCHER=
# If debugging is enabled propagate that through to sub-shells
if [[ "$-" == *x* ]]; then
  LAUNCHER="bash -x"
fi

for worker in `cat "$HOSTLIST"|sed  "s/#.*$//;/^$/d"`; do
  echo -n "Connection to $worker as $USER... "
  ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $TACHYON_HOME; rm $TACHYON_HOME/logs/*"
  ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $JMX_HOME; rm $JMX_HOME/mmlogs/*"
  ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $TACHYON_HOME; rm $TACHYON_HOME/gclogs/*"
  sleep 0.02
done

wait
