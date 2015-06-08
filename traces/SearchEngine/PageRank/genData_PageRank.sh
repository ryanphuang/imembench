#!/bin/bash
##
# Use the Google dataset, Run PageRank-plain, a PageRank calculation algorithm on hadoop.
# Need HADOOP and PEGASUS
# To prepare and generate data:
# ./genData_PageRank.sh
# To run:
# ./run_PageRank.sh [#_of_nodes] [#_of_reducers] [makesym or nosym] [#_Iterations_of_GenGragh]
##

echo "Generate PageRank data"

WORK_DIR=`pwd`       

mkdir $WORK_DIR/data-PageRank
read -p "Please Enter The Iterations of GenGragh: " I

echo "WORK_DIR=$WORK_DIR data will be generated in $WORK_DIR/data-PageRank"

#/////////////////////////////////////////////////////////////////////////////
#Parameters:
# -o:Output graph file name (default:'graph.txt')
# -m:Matrix (in Maltab notation) (default:'0.9 0.5; 0.5 0.1')
# -i:Iterations of Kronecker product (default:5)
# -s:Random seed (0 - time seed) (default:0)
#/////////////////////////////////////////////////////////////////////////////

../BigDataGeneratorSuite/Graph_datagen/gen_kronecker_graph  -o:$WORK_DIR/data-PageRank/Google_genGraph_$I.txt -m:"0.8305 0.5573; 0.4638 0.3021" -i:$I

head -4 $WORK_DIR/data-PageRank/Google_genGraph_$I.txt > $WORK_DIR/data-PageRank/Google_parameters_$I
sed 1,4d $WORK_DIR/data-PageRank/Google_genGraph_$I.txt > $WORK_DIR/data-PageRank/Google_genGraph_$I.tmp

mv $WORK_DIR/data-PageRank/Google_genGraph_$I.tmp $WORK_DIR/data-PageRank/Google_genGraph_$I.txt

$HADOOP_HOME/bin/hadoop fs -rmr $WORK_DIR/data-PageRank/Google_genGraph_$I.txt

echo "copyFromLocal To HDFS!"
echo "$WORK_DIR/data-PageRank"
$HADOOP_HOME/bin/hadoop fs -copyFromLocal $WORK_DIR/data-PageRank/Google_genGraph_$I.txt $WORK_DIR/data-PageRank/Google_genGraph_$I.txt





