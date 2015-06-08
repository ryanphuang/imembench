#!/bin/bash
echo "number of centers K:"
read K
echo "max_iterations m:"
read m
spark-submit --class cn.ac.ict.bigdatabench.KMeans $JAR_FILE /Spark-Kmeans/data-Kmeans ${K} ${m}
