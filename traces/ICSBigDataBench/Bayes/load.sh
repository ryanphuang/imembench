#/bin/bash
##
# Use the amazon naivebayes dataset, trains and tests a classifier.
# Need HADOOP and MAHOUT
# To prepare and generate data:
# ./genData_naivebayes.sh
# To run:
# ./run_naivebayes.sh
##
 
WORK_DIR=`pwd`

#bayes-trainmodel
cd data-naivebayes/
${HADOOP_HOME}/bin/hdfs dfs -mkdir $1/Bayes
${HADOOP_HOME}/bin/hdfs dfs -rm -r $1/Bayes/model
${HADOOP_HOME}/bin/hdfs dfs -put model $1/Bayes

${HADOOP_HOME}/bin/hdfs dfs -rm -r $1/Bayesclassifier/testdata
${HADOOP_HOME}/bin/hdfs dfs -mkdir $1/Bayesclassifier
${HADOOP_HOME}/bin/hdfs dfs -copyFromLocal ${WORK_DIR}/data-naivebayes/testdata $1/Bayesclassifier
echo "Finished copyFromLocal"


