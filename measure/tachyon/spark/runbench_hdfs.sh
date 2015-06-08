#!/bin/bash

HDFS_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER/data-MicroBenchmarks

HDFS_PREFIX=hdfs://$HDFS_MASTER:9000$PREFIX

RAW_FILES=$HDFS_PREFIX/in
SORT_FILES=$HDFS_PREFIX/sort-out
TSORT_FILE=$HDFS_PREFIX/tsort-file

HDFS_OUTPUT=$HDFS_PREFIX/out/spark/hdfs

if [ $# -ne 1 ]
then
    echo "please specify the workload to run (pagerank|grep|terasort)'"
else
    echo "START RUN "$1

    if [ $1 == "wordcount" ];then
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/wordcount
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.WordCount bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES $HDFS_OUTPUT/wordcount

    elif [ $1 == "grep" ];then
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/grep
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.Grep bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES "dfs[a-z.]+" $HDFS_OUTPUT/grep

    elif [ $1 == "sort" ];then
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/sort
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.Sort bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES $HDFS_OUTPUT/sort

    else
        echo "Sorry we do not support this benchmark ("$WORKLOAD")"
    fi
    #$HADOOP_HOME/bin/hdfs dfs -rm -r $HDFS_OUTPUT
fi
