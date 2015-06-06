#!/bin/bash

###TODO: export
#HDFS_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER

#$HADOOP_HOME/bin/hadoop dfs -rm -r $WC_INPUT
#$HADOOP_HOME/bin/hadoop dfs -copyFromLocal XXX $WC_INPUT/big.txt 

#HADOOP_HOME=/mnt/hadoop/hadoop-2.5.2

HADOOP_TRACE_HOME=/mnt/imembench/traces/SearchEngine-Hadoop-2.2.0
HADOOP_TRACE_BIN=$HADOOP_TRACE_HOME/MicroBenchmarks
HADOOP_TRACE_DATA=$HADOOP_TRACE_BIN/data-MicroBenchmarks/in

cd $HADOOP_TRACE_BIN

#1. Generate the workload 
./genData_MicroBenchmarks.sh

#2. Upload to HDFS
${HADOOP_HOME}/bin/hadoop fs -rm -r data-MicroBenchmarks
${HADOOP_HOME}/bin/hadoop fs -mkdir data-MicroBenchmarks
${HADOOP_HOME}/bin/hadoop fs -copyFromLocal ${HADOOP_TRACE_DATA} data-MicroBenchmarks/in

#3. Do the seq for sort operation
${HADOOP_HOME}/bin/hadoop fs -rm -r data-MicroBenchmarks/sort-out
${HADOOP_HOME}/bin/hadoop fs -mkdir data-MicroBenchmarks/sort-out
${HADOOP_HOME}/bin/hadoop jar ${HADOOP_TRACE_HOME}/sort-transfer/ToSeqFile.jar ToSeqFile data-MicroBenchmarks/in data-MicroBenchmarks/sort-out

