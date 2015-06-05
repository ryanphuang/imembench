#!/bin/bash

#TODO
FILE10=big.txt

###TODO: export
HADOOP_HOME=/mnt/hadoop/hadoop-2.5.2

HDFS_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER

HDFS_PREFIX=hdfs://$HDFS_MASTER:9000$PREFIX

if [ $# -ne 1 ]
then
    echo "please specify the workload to run (wordcount|grep|terasort)'"
else
    WORKLOAD=$1
   
    HDFS_OUTPUT=$HDFS_PREFIX/$WORKLOAD"_output"
    HDFS_INPUT_DIR=$HDFS_PREFIX/$WORKLOAD"_input"

    echo $HDFS_OUTPUT  
    $HADOOP_HOME/bin/hdfs dfs -rm -r $HDFS_OUTPUT
    
    if [ $WORKLOAD == "wordcount" ]
    then
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar wordcount $HDFS_INPUT_DIR/$FILE10 $HDFS_OUTPUT

    elif [ $WORKLOAD == "grep" ]
    then
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar grep $HDFS_INPUT_DIR/$FILE10 $HDFS_OUTPUT "dfs[a-z.]+"

    elif [ $WORKLOAD == "terasort" ]
    then
        echo "START RUN "$1
        TSORT_FILE="tsort_data"
        $HADOOP_HOME/bin/hdfs dfs -rm $HDFS_INPUT_DIR/$TSORT_FILE
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar teragen 1000 $HDFS_INPUT_DIR/$TSORT_FILE
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar terasort $HDFS_INPUT_DIR/$TSORT_FILE $HDFS_OUTPUT
    else
        echo "Sorry we do not support this benchmark ("$WORKLOAD")"
    fi
    $HADOOP_HOME/bin/hdfs dfs -rm -r $HDFS_OUTPUT
fi
