#!/bin/bash

#TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4

TACHYON_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER/ICSBigDataBench

TACHYON_PREFIX=tachyon://$TACHYON_MASTER:19998$PREFIX

RAW_FILES=$TACHYON_PREFIX/data-MicroBenchmarks/in
SORT_FILES=$TACHYON_PREFIX/data-MicroBenchmarks/sort-out
TSORT_FILE=$TACHYON_PREFIX/data-MicroBenchmarks/tsort-file

BAYES_TESTDATA=$TACHYON_PREFIX/Bayesclassifier/testdata
BAYES_MODEL=$TACHYON_PREFIX/Bayes/model

KMEANS_DATA=$TACHYON_PREFIX/Spark-Kmeans/data-Kmeans

CC_DATA=$TACHYON_PREFIX/Connect-data/Facebook_genGragh.txt

TACHYON_OUTPUT=$TACHYON_PREFIX/out/spark/tachyon

if [ $# -ne 1 ]
then
    echo "please specify the workload to run (wordcount|grep|sort|bayes|kmeans|cc)'"
else
    echo "START RUN "$1

    if [ $1 == "wordcount" ];then
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/wordcount
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.WordCount bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES $TACHYON_OUTPUT/wordcount

    elif [ $1 == "grep" ];then
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/grep
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.Grep bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES "dfs[a-z.]+" $TACHYON_OUTPUT/grep

    elif [ $1 == "sort" ];then
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/sort
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.Sort bigdatabench-spark_2.10-1.1.0.jar $RAW_FILES $TACHYON_OUTPUT/sort
    
    elif [ $1 == "bayes" ];then
        $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/bayes
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.NaiveBayesClassifier bigdatabench-spark_2.10-1.1.0.jar $BAYES_TESTDATA $BAYES_MODEL $TACHYON_OUTPUT/bayes

    elif [ $1 == "kmeans" ];then
        #$TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/kmeans
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.KMeans bigdatabench-spark_2.10-1.1.0.jar $KMEANS_DATA 10 10 
   
    elif [ $1 == "cc" ];then
        #$TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT/cc
        $SPARK_HOME/bin/spark-submit --class cn.ac.ict.bigdatabench.ConnectedComponent bigdatabench-spark_2.10-1.1.0.jar $CC_DATA 
    else
        echo "Sorry we do not support this benchmark ("$WORKLOAD")"
    fi
    #$TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT
fi
