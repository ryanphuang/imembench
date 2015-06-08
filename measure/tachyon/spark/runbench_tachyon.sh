#!/bin/bash

#TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4

TACHYON_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER/data-MicroBenchmarks

TACHYON_PREFIX=tachyon://$TACHYON_MASTER:19998$PREFIX

RAW_FILES=$TACHYON_PREFIX/in
SORT_FILES=$TACHYON_PREFIX/sort-out
TSORT_FILE=$TACHYON_PREFIX/tsort-file

TACHYON_OUTPUT=$TACHYON_PREFIX/out/spark/tachyon

if [ $# -ne 1 ]
then
    echo "please specify the workload to run (pagerank|grep|terasort)'"
else
    echo "START RUN "$1

    if [ $1 == "wordcount" ];then
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/wordcount
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.WordCount bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES $TACHYON_OUTPUT/wordcount

    elif [ $1 == "grep" ];then
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/grep
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.Grep bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES "dfs[a-z.]+" $TACHYON_OUTPUT/grep

    elif [ $1 == "sort" ];then
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/sort
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.Sort bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES $TACHYON_OUTPUT/sort

    else
        echo "Sorry we do not support this benchmark ("$WORKLOAD")"
    fi
    #$TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT
fi
