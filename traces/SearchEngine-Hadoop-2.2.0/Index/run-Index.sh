WORK_DIR=`pwd`

echo "Please write you IP:"
read IP
java -jar indexdata.jar ${WORK_DIR}/result hdfs://$IP:9000${WORK_DIR}/data-Index
