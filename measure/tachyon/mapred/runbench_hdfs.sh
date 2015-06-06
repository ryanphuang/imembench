#!/bin/bash

#TODO: redundancy here

###TODO: export
HADOOP_HOME=/mnt/hadoop/hadoop-2.5.2

HDFS_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER/data-MicroBenchmarks

RAW_FILES=$PREFIX/in
SORT_FILES=$PREFIX/sort-out
HDFS_OUTPUT=$PREFIX/out

HDFS_PREFIX=hdfs://$HDFS_MASTER:9000$PREFIX

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
        $HADOOP_HOME/bin/hadoop jar ${HADOOP_HOME}/share/hadoop/mapreduce/hadoop-mapreduce-examples-*.jar sort $SORT_FILES $HDFS_OUTPUT/sort

    elif [ $1 == "terasort" ]; then
        echo "START RUN "$1
        TSORT_FILE="tsort_data"
        $HADOOP_HOME/bin/hdfs dfs -rm $HDFS_INPUT_DIR/$TSORT_FILE
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar teragen 1000 $HDFS_INPUT_DIR/$TSORT_FILE
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar terasort $HDFS_INPUT_DIR/$TSORT_FILE $HDFS_OUTPUT
    else
        echo "Sorry we do not support this benchmark ("$1")"
    fi
    #$HADOOP_HOME/bin/hdfs dfs -rm -r $HDFS_OUTPUT
fi
