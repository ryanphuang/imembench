#!/bin/bash
##
# Use the Facebook dataset, Run HCC, a connected component algorithm on hadoop.
# Need HADOOP and PEGASUS
# To prepare and generate data:
# ./genData_connectedComponents.sh
# To run:
# ./run_connectedComponents.sh [#_of_nodes] [#_of_reducers] [#_Iterations_of_GenGragh]
##

echo "Generate Connected_Components data"

WORK_DIR=`pwd`       

mkdir $WORK_DIR/data-Connected_Components
read -p "Please Enter The Iterations of GenGragh: " I

echo "WORK_DIR=$WORK_DIR data will be generated in $WORK_DIR/data-Connected_Components"

#/////////////////////////////////////////////////////////////////////////////
#Parameters:
# -o:Output graph file name (default:'graph.txt')
# -m:Matrix (in Maltab notation) (default:'0.9 0.5; 0.5 0.1')
# -i:Iterations of Kronecker product (default:5)
# -s:Random seed (0 - time seed) (default:0)
#/////////////////////////////////////////////////////////////////////////////

../BigDataGeneratorSuite/Graph_datagen/gen_kronecker_graph  -o:$WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.txt -m:"0.8301 0.558; 0.4638 0.3019" -i:$I

head -4 $WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.txt > $WORK_DIR/data-Connected_Components/Facebook_parameters_$I
sed 1,4d $WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.txt > $WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.tmp

mv $WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.tmp $WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.txt

$HADOOP_HOME/bin/hadoop fs -rmr $WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.txt

echo "copyFromLocal To HDFS!"
echo "$WORK_DIR/data-Connected_Components"
$HADOOP_HOME/bin/hadoop fs -copyFromLocal $WORK_DIR/data-Connected_Components/Facebook_genGragh_$I.txt /Connect-data/Facebook_genGragh_$I.txt


