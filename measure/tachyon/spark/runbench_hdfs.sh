#!/bin/bash

HDFS_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER/ICSBigDataBench

HDFS_PREFIX=hdfs://$HDFS_MASTER:9000$PREFIX

RAW_FILES=$HDFS_PREFIX/data-MicroBenchmarks/in
SORT_FILES=$HDFS_PREFIX/data-MicroBenchmarks/sort-out
TSORT_FILE=$HDFS_PREFIX/data-MicroBenchmarks/tsort-file

BAYES_TESTDATA=$HDFS_PREFIX/Bayesclassifier/testdata
BAYES_MODEL=$HDFS_PREFIX/Bayes/model

KMEANS_DATA=$HDFS_PREFIX/Spark-Kmeans/data-Kmeans

CC_DATA=$HDFS_PREFIX/Connect-data/Facebook_genGragh.txt

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
    elif [ $1 == "bayes" ];then
        $HADOOP_HOME/bin/hadoop fs -rm -r $HDFS_OUTPUT/bayes
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.NaiveBayesClassifier bigdatabench-spark_2.10-1.1.0.jar $BAYES_TESTDATA $BAYES_MODEL $HDFS_OUTPUT/bayes

    elif [ $1 == "kmeans" ];then
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.KMeans bigdatabench-spark_2.10-1.1.0.jar $KMEANS_DATA 10 10 
   
    elif [ $1 == "cc" ];then
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.ConnectedComponent bigdatabench-spark_2.10-1.1.0.jar $CC_DATA 

    else
        echo "Sorry we do not support this benchmark ("$WORKLOAD")"
    fi
    #$HADOOP_HOME/bin/hdfs dfs -rm -r $HDFS_OUTPUT
fi
