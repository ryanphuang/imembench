#!/bin/bash

#TODO
FILE10=big.txt

###TODO: export
TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4

TACHYON_MASTER=ccied6.sysnet.ucsd.edu

USER=`whoami`
PREFIX=/user/$USER

TACHYON_PREFIX=tachyon://$TACHYON_MASTER:19998$PREFIX

if [ $# -ne 1 ]
then
    echo "please specify the workload to run (wordcount|grep|terasort)'"
else
    WORKLOAD=$1
    
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
    $TACHYON_HOME/bin/tachyon tfs rmr $TACHYON_OUTPUT
fi
