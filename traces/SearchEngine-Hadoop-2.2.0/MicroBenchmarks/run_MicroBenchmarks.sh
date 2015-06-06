#!/bin/bash
##
# Micro Benchmarks Workload: sort, grep, wordcount
# Need HADOOP 
# To prepare and generate data:
#  ./genData_MicroBenchmarks.sh
# To run:  
#  ./run_MicroBenchmarks.sh
##

if [ ! -e $HADOOP ]; then
  echo "Can't find hadoop in $HADOOP, exiting"
  exit 1
fi

WORK_DIR=`pwd`
echo "WORK_DIR=$WORK_DIR data should be put in $WORK_DIR/data-MicroBenchmarks/in"

algorithm=( sort grep wordcount)
if [ -n "$1" ]; then
  choice=$1
else
  echo "Please select a number to choose the corresponding Workload algorithm"
  echo "1. ${algorithm[0]} Workload"
  echo "2. ${algorithm[1]} Workload"
  echo "3. ${algorithm[2]} Workload"
  read -p "Enter your choice : " choice
fi

echo "ok. You chose $choice and we'll use ${algorithm[$choice-1]} Workload"
Workloadtype=${algorithm[$choice-1]} 

if [ "x$Workloadtype" == "xsort" ]; then
  ${HADOOP_HOME}/bin/hadoop fs -rmr ${WORK_DIR}/data-MicroBenchmarks/out/sort
  ${HADOOP_HOME}/bin/hadoop jar  ${HADOOP_HOME}/share/hadoop/mapreduce/hadoop-mapreduce-examples-*.jar  sort /sort-out  ${WORK_DIR}/data-MicroBenchmarks/out/sort

elif [ "x$Workloadtype" == "xgrep" ]; then

  ${HADOOP_HOME}/bin/hadoop fs -rmr ${WORK_DIR}/data-MicroBenchmarks/out/grep
  ${HADOOP_HOME}/bin/hadoop jar  ${HADOOP_HOME}/share/hadoop/mapreduce/hadoop-mapreduce-examples-*.jar grep ${WORK_DIR}/data-MicroBenchmarks/in ${WORK_DIR}/data-MicroBenchmarks/out/grep a*xyz

elif [ "x$Workloadtype" == "xwordcount" ]; then

  ${HADOOP_HOME}/bin/hadoop fs -rmr ${WORK_DIR}/data-MicroBenchmarks/out/wordcount
  ${HADOOP_HOME}/bin/hadoop jar  ${HADOOP_HOME}/share/hadoop/mapreduce/hadoop-mapreduce-examples-*.jar  wordcount  ${WORK_DIR}/data-MicroBenchmarks/in ${WORK_DIR}/data-MicroBenchmarks/out/wordcount

  echo "unknown cluster type: $clustertype"
fi 
