#!/bin/bash

HOSTLIST="./peers"

#Collect mm logs
for worker in `cat "$HOSTLIST"|sed  "s/#.*$//;/^$/d"`; do
  scp -r $worker:/mnt/tixu/JMXMM223/mmlogs ./tmp
  mv ./tmp/mmlogs/* ./mmlogs/
  rm -Rf ./tmp/mmlogs
  sleep 0.02
done

#TODO: Collect the GC logs
#while read line           
#do
#    #    mv ./tmp/logs/* ./logs/
#    rm -Rf ./tmp/logs   
#done <$NODELST
