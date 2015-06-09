#Basic Settings
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

#Start Tachyon

Current, we support the following model of Tachyon cluster

- `clean`: start a out-of-box Tachyon cluster without any logging overhead (used for performance measurement)
- `mm`: enable the memory utilization monitoring (via JMX) and GC logging
- `inst`: using the instrumented Tachyon worker which will log the memory allocation events 

The commands associated with the three models are
```
$ ./tconf.sh {clean, mm, inst}                
$ ./tachyon_launcher start-{clean, mm, inst}  
$ ./tachyon_launcher stop-{clean, mm, inst}
```
[NOTE] Make sure you do the `tconf.sh` first before launching any of the models. `tconf.sh` will copy the corresponding scripts, configuration files, and JARs to each node.

#Run workloads
Therea are two subdirs, `spark` and `mapred` containing the scripts to run the Spark and MapReduce workloads correspondingly. The scripts ars:
- `mapred/runbench_hdfs.sh` and `mapred/runbench_tachyon.sh`: run ICS benchmarks on `HDFS` and `Tachyon` respectively. You can run the following ones: `wordcount`, `grep`, `sort`, and `terasort`.
- `spark/runbench_hdfs.sh` and `spark/runbench_tachyon.sh`: run ICS benchmarks on `HDFS` and `Tachyon` respectively. You can run the following ones: `wordcount`, `grep`, `sort`, `bayes`, `kmeans`, and `cc`.

For example, to run a spark sort benchmark on top of Tachyon, you can do,
```
$ cd spark
$ ./runbench_tachyon sort     
```

#Processes
Using `mm` as the example, I expect the measurement follows the following process:
```
$ ./tconf.sh mm                      //initialize the configurations for memory monitoring
$ ./tachyon_launch.sh start-mm       //restart tachyon and the memory monitor
$ cd spark  
$ ./runbench_tachyon wordcount       //running workloads
$ ./runbench_tachyon sort  
$ ./runbench_tachyon kmeans  
$ ......
$ ./tachyon_mgr.sh stop-mm           //stop tachyon and copy all the logs into the local mmlogs and gclogs dirs
```

#Other scripts

You will see the following scripts:
- `sync.sh`: synchronize a file on the current file system to all the workers
- `prepare.sh`: prepare the ICS BigDataBench and load into HDFS. !!!!!*You should not need to run this unless you format the HDFS.*!!!!!
- `gccollector.sh`, ``jmxmonitor`, `alloccollector.sh`, and `logcleaner.sh`: collect different types of logs from all the workers and store in the local dirs. !!!!!*You should not need to run this because it is integrated into `tachyon_mgr.sh`*!!!!!

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

The settings will automatically be copied by `tconf.sh` and the JMX monitors will be started by `tachyon_launcher`.

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
AFter starting `Tachyon`, you can find GC logs at `$TACHYON_HOME/gclogs/gclog.worker$SUFFIX` (I added `$SUFFIX` to identify the host and start time of the gclog).

The settings will automatically be copied by `tconf.sh` and the JMX monitors will be started by `tachyon_launcher`.

#About memory allocation overhead 

Obtainning memory allocation overhead requires instrumentation of the Tachyon workers so we need to recompile the Tachyon worker code (we use `java-allocation-instrumenter-3.0.jar` to do the instrumentation). Also, you need to change the startup scripts to include a Java agent (the standard way to do instrumentation in Java).

I have prepared an instrumened version of Tachyon JAR and the startup scripts needed. Again, the settings will automatically be copied by `tconf.sh`.

#Start/stop the clusters in normal way
If you want to start and stop the cluters, especially the other components like HDFS, YARN, and Spark. You can use the following commands. Make sure you first stop the upper-level systems and stop `HDFS` last (`Tachyon` and `YARN` both are connected to `HDFS`).

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

