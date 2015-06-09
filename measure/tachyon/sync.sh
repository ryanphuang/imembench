#!/usr/bin/env bash

LAUNCHER=
# If debugging is enabled propagate that through to sub-shells
if [[ "$-" == *x* ]]; then
  LAUNCHER="bash -x"
fi

if [ $# -ne 1 ]
then
    echo "please specify the file path to deploy'"
fi

NODELST="./peers"

for worker in `cat $NODELST`; do
    path=`dirname $1`/`basename $1`
    echo "copy "$path" to "$worker
    scp $path "$worker":$path
done
