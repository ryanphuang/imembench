#!/bin/bash

WORKERLST="./peers"

if [ $# -ne 1 ]
then
    echo "please specify the file path to deploy'"
else
    while read line           
    do
        echo "copy "$1" to "$line
        path=`dirname $1`/`basename $1`
        echo $path
        scp $path "$line":$path
    done <$WORKERLST
fi
