#!/bin/bash
##
# Use the WIKI dataset, trains and tests a cluster.
# Need HADOOP and MAHOUT
# To prepare and generate data:
# ./genData_Kmeans.sh
# To run:
# ./run_Kmeans.sh
##

${HADOOP_HOME}/bin/hdfs dfs -rm -r $1/Spark-Kmeans
${HADOOP_HOME}/bin/hdfs dfs -mkdir $1/Spark-Kmeans
${HADOOP_HOME}/bin/hdfs dfs -copyFromLocal data-Kmeans $1/Spark-Kmeans
echo "Finished copyFromLocal"

