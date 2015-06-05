#!/bin/bash

WORKERLST="./peers"

if [ $# -ne 1 ]
then
    echo "please specify the file path to deploy'"
else
    while read line           
    do
        echo "copy "$1" to "$line
        path=`pwd -P $1`/$1
        echo $path
        scp $path "$line":$path
    done <$WORKERLST
fi
