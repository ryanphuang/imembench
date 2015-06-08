#!/bin/bash
echo "Internation I:"
read I
spark-submit --class cn.ac.ict.bigdatabench.ConnectedComponent $JAR_FILE /Connect-data/Facebook_genGragh_$I.txt
