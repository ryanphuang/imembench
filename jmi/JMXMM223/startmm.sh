#!/bin/bash
if [ $# -ne 1 ]
then
    echo "please specify either 'master' or 'worker'"
else
    if [ "$1" = "master" ]
    then
        echo "Start recording mm usage of the local master"
        java -jar target/jmxmm.jar -p 19997
    elif [ "$1" == "worker" ]
    then
        echo "Start recording mm usage of the local worker"
        java -jar target/jmxmm.jar -p 29997
    else
        echo "[ERROR] unknown role"
    fi
fi
