#!/usr/bin/env bash

HOSTLIST="./peers"

LAUNCHER=
# If debugging is enabled propagate that through to sub-shells
if [[ "$-" == *x* ]]; then
  LAUNCHER="bash -x"
fi

if [ $# -ne 1 ]
then
  echo "Option is clean|mm|inst"
  exit 1
fi

for worker in `cat "$HOSTLIST"|sed  "s/#.*$//;/^$/d"`; do
  echo -n "Connection to $worker as $USER... "
  ./logcleaner.sh

  if [ $1 == "clean" ]; then
    scp tconf/tachyon-env.sh.clean $worker:$TACHYON_HOME/conf/tachyon-env.sh
    scp tconf/tachyon-start.sh.org $worker:$TACHYON_HOME/bin/tachyon-start.sh
    scp tconf/tachyon-0.6.4-jar-with-dependencies.jar.org $worker:$TACHYON_HOME/core/target/tachyon-0.6.4-jar-with-dependencies.jar
  elif [ $1 == "mm" ]; then
    scp tconf/tachyon-env.sh.mm $worker:$TACHYON_HOME/conf/tachyon-env.sh
    scp tconf/tachyon-start.sh.org $worker:$TACHYON_HOME/bin/tachyon-start.sh
    scp tconf/tachyon-0.6.4-jar-with-dependencies.jar.org $worker:$TACHYON_HOME/core/target/tachyon-0.6.4-jar-with-dependencies.jar
  elif [ $1 == "inst" ]; then
    scp tconf/tachyon-env.sh.clean $worker:$TACHYON_HOME/conf/tachyon-env.sh
    scp tconf/tachyon-start.sh.inst $worker:$TACHYON_HOME/bin/tachyon-start.sh
    scp tconf/tachyon-0.6.4-jar-with-dependencies.jar.inst $worker:$TACHYON_HOME/core/target/tachyon-0.6.4-jar-with-dependencies.jar
  else
    echo "Option is clean|mm|inst"
  fi
  sleep 0.02
done

wait
