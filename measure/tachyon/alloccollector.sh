#!/usr/bin/env bash

TACHYON_HOME="/mnt/tachyon/tachyon-0.6.4"
HOSTLIST="./peers"

LAUNCHER=
# If debugging is enabled propagate that through to sub-shells
if [[ "$-" == *x* ]]; then
  LAUNCHER="bash -x"
fi

for worker in `cat "$HOSTLIST"|sed  "s/#.*$//;/^$/d"`; do
  echo -n "Connection to $worker as $USER... "
  scp -r $worker:$TACHYON_HOME/logs ./tmp
  mv ./tmp/logs/* ./alloclogs/
  rm -Rf ./tmp/logs
  
  ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker "cd $TACHYON_HOME; rm $TACHYON_HOME/logs/*"
  sleep 0.02
done

wait
