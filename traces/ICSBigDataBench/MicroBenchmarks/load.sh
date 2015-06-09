#!/bin/bash

#HADOOP_TRACE_BIN=$HADOOP_TRACE_HOME/MicroBenchmarks
#HADOOP_TRACE_DATA=$HADOOP_TRACE_BIN/data-MicroBenchmarks/in

${HADOOP_HOME}/bin/hdfs dfs -rm -r $1/data-MicroBenchmarks
${HADOOP_HOME}/bin/hdfs dfs -mkdir $1/data-MicroBenchmarks
#${HADOOP_HOME}/bin/hdfs dfs -mkdir $1/data-MicroBenchmarks/in

${HADOOP_HOME}/bin/hdfs dfs -copyFromLocal data-MicroBenchmarks/in $1/data-MicroBenchmarks

#3. Do the seq for sort operation
${HADOOP_HOME}/bin/hdfs dfs -rm -r $1/data-MicroBenchmarks/sort-out
${HADOOP_HOME}/bin/hdfs dfs -mkdir $1/data-MicroBenchmarks/sort-out
${HADOOP_HOME}/bin/hadoop jar ../sort-transfer/ToSeqFile.jar ToSeqFile $1/data-MicroBenchmarks/in $1/data-MicroBenchmarks/sort-out

#$HADOOP_HOME/bin/hadoop fs -rm -r data-MicroBenchmarks/tsort-file
#$HADOOP_HOME/bin/hadoop jar ${HADOOP_HOME}/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar teragen 1000000 data-MicroBenchmarks/tsort-file


