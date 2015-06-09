#!/bin/bash

WORKERLST="./peers"

if [ $# -ne 1 ]
then
    echo "please specify the file path to deploy'"
else
    while read line           
    do
        path=`dirname $1`/`basename $1`
        echo "copy "$path" to "$line
        scp $path "$line":$path
    done <$WORKERLST
fi
