#Basic
###a. Login as `cse223_tixu` 
(otherwise you may have permission issues)

###b. Make sure the following environmental variables are set:
(If you login as `cse223_tixu`, you should have them set as they are specified in `.bashrc` and `.bash_profile`)
```
export JAVA_HOME=/usr/lib/jvm/java-1.7.0-openjdk-amd64
export HADOOP_HOME=/mnt/hadoop/hadoop-2.5.2/
export SPARK_HOME=/mnt/spark/spark-master/
export TACHYON_HOME=/mnt/tachyon/tachyon-0.6.4/
```

###c. Login to `ccied6` and switch to the measure directory
(all the scripts you need to operate are located here)
```
cd /mnt/imembench/measure/tachyon
```

#Useful scripts

You probably want to use the following scripts:
- `tachyon_mgr.sh`: Help you start and stop `Tachyon` servers and collect the memory utilization logs (mmlogs) and GC logs (gclogs). If you don't want to collect these logs, start `Tachyon` in the normal way (see below).
- `mapred/runbench_hdfs.sh` and `mapred/runbench_tachyon.sh`: run ICS benchmarks on `HDFS` and `Tachyon` respectively. You can run the following ones: `wordcount`, `grep`, `sort`, and `terasort`.
- `spark/runbench_hdfs.sh` and `spark/runbench_tachyon.sh`: run ICS benchmarks on `HDFS` and `Tachyon` respectively. You can run the following ones: `wordcount`, `grep`, `sort`, `bayes`, `kmeans`, and `cc`.
- `sync.sh`: synchronize a file on the current file system to all the workers
- `prepare.sh`: prepare the ICS BigDataBench and load into HDFS. !!!!!*You should not need to run this unless you format the HDFS.*!!!!!
- `gccollector.sh`: collect the GC logs from all the workers into the local gclogs dir. !!!!!*You should not need to run this because it is integrated into `tachyon_mgr.sh`*!!!!!
- `jmxmonitor.sh`: scripts to manage the memory utlization monitor. !!!!!*You should not need to run this because it is integrated into `tachyon_mgr.sh`*!!!!!

The process of running the scripts is supposed to be,
```
$ ./tachyon_mgr.sh start         //restart tachyon and the memory monitor
$ ./spark/runbench_hdfs sort     //run the spark sort workload
$ ./tachyon_mgr.sh stop          //stop tachyon and copy all the logs into the local mmlogs and gclogs dirs
```
Note: the ICS BigDataBench is located in `/mnt/imembench/traces/ICSBigDataBench/`, but you should not need to change it.

#About memory utilization logs (`mmlogs`)

`mmlogs` are collected through the JMX (Java Management Extensions) interface. To enable JMX for `Tachyon`, you need to
have the following settings
```
export TACHYON_JAVA_OPTS+="
  ...
  -Dcom.sun.management.jmxremote
  -Dcom.sun.management.jmxremote.authenticate=false
  -Dcom.sun.management.jmxremote.ssl=false
  ...
"
export TACHYON_WORKER_JAVA_OPTS="
  $TACHYON_JAVA_OPTS
  -Dcom.sun.management.jmxremote.port=29997
"
```
Every node has a JMI client in ```/mnt/tixu/JMXMM223/```, and there is scripts to start (```startmm.sh```) and stop (```stopmm.sh```) the monitor. 

#About GC logs (gclogs)
`gclogs` can be turned on with the following settings:
```
export TACHYON_JAVA_OPTS+="
  ...
  -XX:+PrintGCDetails
  -XX:+PrintGCDateStamps
  ...
"
export TACHYON_WORKER_JAVA_OPTS="
  $TACHYON_JAVA_OPTS
  -Xloggc:$TACHYON_HOME/gclogs/gclog.worker$SUFFIX
"
```
AFter starting `Tachyon`, you can find GC logs at `$TACHYON_HOME/gclogs/gclog.worker$SUFFIX` (I added `$SUFFIX` to identify the host and start time of the gclog)

#About memory allocation overhead 

This requires instrumentation of the Tachyon workers so you need to recompile the Tachyon worker code. Also, you need to change the startup scripts. Therefore, do this at the last measurement step, 

I will prepare a compiled version of Tachyon jar and write some scripts tonight to automatically replace stuff.


#Start/stop the clusters in normal way
First, make sure you stop the cluster before you start them. Also, make sure you first stop the upper-level systems and stop `HDFS` last (`Tachyon` and `YARN` both are connected to `HDFS`).

###Start
```
$SPARK_HOME/sbin/start-all.sh
$TACHYON_HOME/bin/tachyon-start.sh all SudoMount
$HADOOP_HOME/sbin/start-yarn.sh
$HADOOP_HOME/sbin/start-dfs.sh
```
###Stop
```
$SPARK_HOME/sbin/stop-all.sh
$TACHYON_HOME/bin/tachyon-stop.sh
$HADOOP_HOME/sbin/stop-yarn.sh
$HADOOP_HOME/sbin/stop-dfs.sh
```

