#!/bin/bash

#TODO: redundancy here

HDFS_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER/data-MicroBenchmarks

HDFS_PREFIX=hdfs://$HDFS_MASTER:9000$PREFIX

RAW_FILES=$HDFS_PREFIX/in
SORT_FILES=$HDFS_PREFIX/sort-out
TSORT_FILE=$HDFS_PREFIX/tsort-file

HDFS_OUTPUT=$HDFS_PREFIX/out/mapred/hdfs


if [ $# -ne 1 ]
then
    echo "please specify the workload to run (wordcount|grep|sort|terasort)'"
else
    if [ $1 == "wordcount" ];then 
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/wordcount
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar wordcount $RAW_FILES $HDFS_OUTPUT/wordcount

    elif [ $1 == "grep" ]; then
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/grep
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar grep $RAW_FILES $HDFS_OUTPUT/grep "dfs[a-z.]+"
    
    elif [ $1 == "sort" ]; then
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/sort
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar sort $SORT_FILES $HDFS_OUTPUT/sort

    elif [ $1 == "terasort" ]; then
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/terasort
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar terasort $TSORT_FILE $HDFS_OUTPUT/terasort
    else
        echo "Sorry we do not support this benchmark ("$1")"
    fi
    #$HADOOP_HOME/bin/hdfs dfs -rm -r $HDFS_OUTPUT
fi
