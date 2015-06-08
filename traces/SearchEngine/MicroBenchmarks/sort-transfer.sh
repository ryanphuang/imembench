#!/bin/bash

#dataset=`echo $1 | tr [a-z] [A-Z]`
#${HADOOP_HOME}/bin/hadoop fs -rmr /user/root/seqFile1/part-m-00000-out

WORK_DIR=`pwd`
${HADOOP_HOME}/bin/hadoop fs -rmr /sort-out
 ${HADOOP_HOME}/bin/hadoop fs -mkdir /sort-out

 #${HADOOP_HOME}/bin/hadoop jar  ${HADOOP_HOME}/ToSeqFile.jar ToSeqFile  <inputfile> <HDF_outputfile>
${HADOOP_HOME}/bin/hadoop jar  ${HADOOP_HOME}/ToSeqFile.jar ToSeqFile  ${WORK_DIR}/data-MicroBenchmarks/in /sort-out
