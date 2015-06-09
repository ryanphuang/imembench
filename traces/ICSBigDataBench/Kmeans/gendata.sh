#!/bin/bash
##
# Use the WIKI dataset, trains and tests a cluster.
# Need HADOOP and MAHOUT
# To prepare and generate data:
# ./genData_Kmeans.sh
# To run:
# ./run_Kmeans.sh
##

echo "Preparing Kmeans data dir"
rm -fr data-Kmeans
./Generating Image_data/color100.txt 200000 > data-Kmeans

#${HADOOP_HOME}/bin/hadoop fs -rmr /Spark-Kmeans
#${HADOOP_HOME}/bin/hadoop fs -mkdir /Spark-Kmeans
#${HADOOP_HOME}/bin/hadoop fs -copyFromLocal data-Kmeans /Spark-Kmeans
#echo "Finished copyFromLocal"

