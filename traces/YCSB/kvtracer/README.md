# kvtracer

Dummy YCSB client to record all the workload operations in a trace file, and convert 
them to simple key-value GET/PUT operation to be used by some other benchmark tools.

## Usage
To compile, put the `kvtracer` directory in `YCSB` source directory and then
`mvn -pl kvtracer -am clean package`. The jar file should be generated in
the `target` directory in `kvtracer`.

Then to generate the workload trace, run
```
bin/ycsb load kvtracer -P workloads/workloada -p "kvtracer.tracefile=tracea_load.txt" -p "kvtracer.keymapfile=tracea_keys.txt"
```
and
```
bin/ycsb run kvtracer -P workloads/workloada -p "kvtracer.tracefile=tracea_run.txt" -p "kvtracer.keymapfile=tracea_keys.txt"
```
Replace the `kvtracer.tracefile` and `kvtracer.keymapfile` configuration if needed.
The keymap file is needed because the YCSB operations are richer than simple key-value
GET/PUT, e.g., delete table with a name will delete all keys/fields for that table. 
We need to know what exactly are these keys.

The final traces are in `tracea_load.txt` (for loading data) and `tracea_run.txt` 
(for operations). You can merge them into one trace.

