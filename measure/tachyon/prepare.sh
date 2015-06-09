#!/bin/bash

USER=`whoami`
PREFIX=/user/$USER/ICSBigDataBench

LOCAL_TRACE_HOME=/mnt/imembench/traces/ICSBigDataBench

if [ $# -ne 1 ]
then
    echo ">>>Please specify the workload to run (micro|bayes|kmeans|cc)'"
    exit 1
else
    if [ $1 == "micro" ];then
        cd $LOCAL_TRACE_HOME/MicroBenchmarks   
    elif [ $1 == "bayes" ];then
        cd $LOCAL_TRACE_HOME/Bayes
    elif [ $1 == "kmeans" ];then
        cd $LOCAL_TRACE_HOME/Kmeans
    elif [ $1 == "cc" ];then
        cd $LOCAL_TRACE_HOME/Connected_Components
    else
        echo ">>>Sorry we do not support this benchmark ("$1")"
        echo ">>>please specify the workload to run (micro|bayes|kmeans|cc|all)'"
        exit 1
    fi
    bash gendata.sh
    bash load.sh $PREFIX
fi
