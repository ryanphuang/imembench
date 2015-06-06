#!/usr/bin/env bash

LAUNCHER=
# If debugging is enabled propagate that through to sub-shells
if [[ "$-" == *x* ]]; then
  LAUNCHER="bash -x"
fi

#bin=`cd "$( dirname "$0" )"; pwd`
#DEFAULT_LIBEXEC_DIR="$bin"/../libexec
#TACHYON_LIBEXEC_DIR=${TACHYON_LIBEXEC_DIR:-$DEFAULT_LIBEXEC_DIR}
#. $TACHYON_LIBEXEC_DIR/tachyon-config.sh

JMX_HOME="/mnt/tixu/JMXMM223"

HOSTLIST="./peers"

for worker in `cat "$HOSTLIST"|sed  "s/#.*$//;/^$/d"`; do
  echo -n "Connection to $worker as $USER... "
  ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $JMX_HOME; $LAUNCHER $JMX_HOME/startmm.sh worker"
  sleep 0.02
done

wait
