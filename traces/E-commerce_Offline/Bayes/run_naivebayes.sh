#!/bin/bash
${HADOOP_HOME}/bin/hadoop fs -rmr /Bayes-result
spark-submit --class cn.ac.ict.bigdatabench.NaiveBayesClassifier $JAR_FILE /Bayesclassifier/testdata /Bayes/model /Bayes-result
  

