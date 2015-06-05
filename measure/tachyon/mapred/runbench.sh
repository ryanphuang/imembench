#!/bin/bash

FILE10=big.txt

###TODO: export
HDFS_MASTER=ccied6.sysnet.ucsd.edu
TACHYON_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER

HDFS_PREFIX=hdfs://$TACHYON_MASTER:9000$PREFIX
TACHYON_PREFIX=tachyon://$TACHYON_MASTER:19998$PREFIX

#$HADOOP_HOME/bin/hadoop dfs -rm -r $WC_INPUT
#$HADOOP_HOME/bin/hadoop dfs -copyFromLocal XXX $WC_INPUT/big.txt 

HADOOP_HOME=/mnt/hadoop/hadoop-2.5.2
TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4

if [ $# -ne 1 ]
then
    echo "please specify the workload to run (wordcount|grep|terasort)'"
else
    
    WORKLOAD=$1
    HDFS_OUTPUT=$HDFS_PREFIX/$WORKLOAD"_output"
    HDFS_INPUT_DIR=$HDFS_PREFIX/$WORKLOAD"_input"
    
    TACHYON_OUTPUT=$TACHYON_PREFIX/$WORKLOAD"_output"
    TACHYON_INPUT_DIR=$TACHYON_PREFIX/$WORKLOAD"_input"

    $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT
    
    if [ $WORKLOAD == "wordcount" ]
    then
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar wordcount -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar $TACHYON_INPUT_DIR/$FILE10 $TACHYON_OUTPUT

    elif [ $WORKLOAD == "grep" ]
    then
        echo "START RUN "$1
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar grep -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar $TACHYON_INPUT_DIR/$FILE10 $TACHYON_OUTPUT "dfs[a-z.]+"

    elif [ $WORKLOAD == "terasort" ]
    then
        echo "START RUN "$1
        TSORT_FILE="tsort_data"
        $TACHYON_HOME/bin/tachyon tfs rm $TACHYON_INPUT_DIR/$TSORT_FILE
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar teragen -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar 1000 $TACHYON_INPUT_DIR/$TSORT_FILE
        
        $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar terasort -libjars $TACHYON_HOME/client/target/tachyon-client-0.6.4-jar-with-dependencies.jar $TACHYON_INPUT_DIR/$TSORT_FILE $TACHYON_OUTPUT
    else
        echo "Sorry we do not support this benchmark ("$WORKLOAD")"
    fi
fi

###USING HDFS
#$HADOOP_HOME/bin/hdfs dfs -rm -r $WC_HDFS_OUTPUT
#$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-2.5.2.jar wordcount $WC_HDFS_INPUT_DIR/$FILE10 $WC_HDFS_OUTPUT


