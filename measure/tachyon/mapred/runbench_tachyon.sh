#!/bin/bash

###TODO: export
TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4

TACHYON_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER/data-MicroBenchmarks

TACHYON_PREFIX=tachyon://$TACHYON_MASTER:19998$PREFIX

RAW_FILES=$TACHYON_PREFIX/in
SORT_FILES=$TACHYON_PREFIX/sort-out
TSORT_FILE=$TACHYON_PREFIX/tsort-file
TACHYON_OUTPUT=$TACHYON_PREFIX/out

if [ $# -ne 1 ]
then
    echo "please specify the workload to run (wordcount|grep|sort|terasort)'"
else
    if [ $1 == "wordcount" ];then
        echo "START RUN "$1
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT_DIR/wordcount
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar wordcount -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar $RAW_FILES $TACHYON_OUTPUT/wordcount

    elif [ $1 == "grep" ];then
        echo "START RUN "$1
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT_DIR/grep
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar grep -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar $RAW_FILES $TACHYON_OUTPUT/grep "dfs[a-z.]+"

    elif [ $1 == "sort" ];then
        echo "START RUN "$1
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT_DIR/sort
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar sort -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar $SORT_FILES $TACHYON_OUTPUT/sort
        
    elif [ $1 == "terasort" ];then
        echo "START RUN "$1
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT_DIR/terasort
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar terasort -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar $TSORT_FILE $TACHYON_OUTPUT/terasort
    else
        echo "Sorry we do not support this benchmark ("$1")"
    fi
    #$TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT
fi
