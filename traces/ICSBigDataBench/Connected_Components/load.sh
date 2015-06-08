#!/bin/bash
##
# Use the Facebook dataset, Run HCC, a connected component algorithm on hadoop.
# Need HADOOP and PEGASUS
# To prepare and generate data:
# ./genData_connectedComponents.sh
# To run:
# ./run_connectedComponents.sh [#_of_nodes] [#_of_reducers] [#_Iterations_of_GenGragh]
##

WORK_DIR=`pwd`       
$HADOOP_HOME/bin/hdfs dfs -rm -r $1/Connect-data/
$HADOOP_HOME/bin/hdfs dfs -mkdir $1/Connect-data/
$HADOOP_HOME/bin/hdfs dfs -copyFromLocal $WORK_DIR/data-Connected_Components/Facebook_genGragh.txt $1/Connect-data/Facebook_genGragh.txt

