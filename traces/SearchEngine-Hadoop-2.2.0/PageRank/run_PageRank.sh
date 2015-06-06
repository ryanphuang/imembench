#!/bin/bash
##
# Use the Google dataset, Run PageRank-plain, a PageRank calculation algorithm on hadoop.
# Need HADOOP and PEGASUS
# To prepare and generate data:
# ./genData_PageRank.sh
# To run:
# ./run_PageRank.sh [#_of_nodes]
##

WORK_DIR=`pwd`

which hadoop > /dev/null
status=$?
if test $status -ne 0 ; then
    echo ""
    echo "Hadoop is not installed in the system."
    echo "Please install Hadoop and make sure the hadoop binary is accessible."
    exit 127 
fi
if [ $# -ne 1 ]; then
    echo 1>&2 Usage: $0 [#_Iterations_of_GenGragh]
    echo 1>&2    ex: $0 10
    exit 127 
fi

let "I=2**$1"
#I=$1
reducers=16
flag=makesym #nosym
echo "[#_of_nodes] : number of nodes in the graph is" $I
echo "[#_of_reducers] : number of reducers to use in hadoop is" $reducers
echo "[makesym or nosym] : makesym-duplicate reverse edges, nosym-use original edge file" $flag
echo "[#_Iterations_of_GenGragh] : Iterations_of_GenGragh is" $1


hadoop dfs -rmr /user/root/pr_tempmv
hadoop dfs -rmr /user/root/pr_output
hadoop dfs -rmr /user/root/pr_minmax
hadoop dfs -rmr /user/root/pr_distr
hadoop dfs -rmr /user/root/pr_vector

#hadoop jar pegasus-2.0.jar pegasus.PagerankNaive $WORK_DIR/data-PageRank/Google_genGraph_$1.txt pr_tempmv pr_output $I $reducers 1024 $flag new
hadoop jar pegasus-2.0.jar pegasus.PagerankNaive -D mapred.input.format.class=org.apache.hadoop.mapred.lib.NLineInputFormat -D mapred.line.input.format.linespermap=2500000 $WORK_DIR/data-PageRank/Google_genGraph_$1.txt pr_tempmv pr_output $I $reducers 1024 $flag new
#hadoop jar pegasus-2.0.jar pegasus.PagerankNaive  /root/ljw/bigdatabench-spark-1.0/SearchEngine/Pagerank/data-PageRank/Google_genGraph_6.txt pr_tempmv pr_output $I $reducers 1024 $flag new

