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

echo "Preparing MicroBenchmarks data dir"

WORK_DIR=`pwd`
echo "WORK_DIR=$WORK_DIR data will be put in $WORK_DIR/data-MicroBenchmarks/in"

mkdir -p ${WORK_DIR}/data-MicroBenchmarks/in

cd ../BigDataGeneratorSuite/Text_datagen/

./gen_text_data.sh lda_wiki1w 20 8000 10000 ${WORK_DIR}/data-MicroBenchmarks/in
#10GB 20 8000 10000
#15GB 30 8000 10000
#30GB 60 8000 10000
cd ../../MicroBenchmarks/

#${HADOOP_HOME}/bin/hadoop fs -copyFromLocal ${WORK_DIR}/data-MicroBenchmarks/in ${WORK_DIR}/data-MicroBenchmarks/in

