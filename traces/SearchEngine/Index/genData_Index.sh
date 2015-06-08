#!/bin/bash
##
# Micro Benchmarks Workload: sort, grep, wordcount
# Need HADOOP 
##

if [ ! -e $HADOOP ]; then
  echo "Can't find hadoop in $HADOOP, exiting"
  exit 1
fi

echo "Preparing MicroBenchmarks data dir"

WORK_DIR=`pwd`
echo "WORK_DIR=$WORK_DIR data will be put in $WORK_DIR/data-Index"

mkdir -p ${WORK_DIR}/data-Index

cd ../BigDataGeneratorSuite/Text_datagen/

./gen_text_data.sh lda_wiki1w 6 80 100 ${WORK_DIR}/data-Index/
#10GB 20 8000 10000
#15G 30 8000 10000
#30G 60 8000 10000
#cd ../../Index/

${HADOOP_HOME}/bin/hadoop fs -copyFromLocal ${WORK_DIR}/data-Index ${WORK_DIR}/data-Index

