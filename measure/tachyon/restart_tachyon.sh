#!/bin/bash

TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4
TACHYON_BIN=$TACHYON_HOME/bin

#1. Stop the monitors

#1. Stop the tachyon cluster
$TACHYON_BIN/tachyon-stop.sh

#2. Format the tachyon cluster
$TACHYON_BIN/tachyon format

#3. Start the tachyon cluster
$TACHYON_BIN/tachyon-start.sh

#4. TODO: start the monitors
