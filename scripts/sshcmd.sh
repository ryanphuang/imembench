#!/usr/bin/env bash

LAUNCHER=
# If debugging is enabled propagate that through to sub-shells
if [[ "$-" == *x* ]]; then
  LAUNCHER="bash -x"
fi

NODELST="./peers"

for worker in `cat $NODELST`; do
    ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no -t $worker $LAUNCHER $"${@// /\\ }" 2>&1
done
