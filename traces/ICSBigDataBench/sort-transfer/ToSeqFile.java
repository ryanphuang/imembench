//package test;
import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.SequenceFile.CompressionType;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.compress.GzipCodec;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;
	 
public class ToSeqFile extends Configured implements Tool {
	 
    public static class Map extends Mapper<LongWritable , Text , BytesWritable, BytesWritable>{
	    	 
	    public void map(LongWritable key,Text value, Context context) throws IOException,InterruptedException{
	    		 
	    	String line=value.toString();
	    		     	
	    	context.write(new BytesWritable(line.getBytes()), new BytesWritable(line.getBytes()));

	    	}
	}
	     
	    
	@Override
	public int run(String[] arg) throws Exception {
	       
	     Configuration conf=getConf();
//	     conf.set("mapred.job.tracker", "172.18.11.71:8001"); 

	     FileSystem fs = FileSystem.get(conf);
	         
	     Job job = new Job(conf, "SeqFile");
	         
	     job.setJarByClass(getClass());
	     //FileInputFormat.setInputPaths(job, "/inputdata/bayes-out/");
	     //Path outDir=new Path("/inputdata/seq-out1");
	     FileInputFormat.setInputPaths(job, arg[0]);
	     Path outDir=new Path(arg[1]);
	     fs.delete(outDir,true);
	     FileOutputFormat.setOutputPath(job, outDir);
	         
	     job.setNumReduceTasks(0);
	
	     job.setMapperClass(Map.class);
			 
		 job.setOutputKeyClass(BytesWritable.class);
	     job.setOutputValueClass(BytesWritable.class);
	
	     job.setOutputFormatClass(SequenceFileOutputFormat.class);
	    
	     SequenceFileOutputFormat.setCompressOutput(job, true);
	 
	     SequenceFileOutputFormat.setOutputCompressorClass(job, GzipCodec.class);
	       
	     SequenceFileOutputFormat.setOutputCompressionType(job, CompressionType.BLOCK);
	 
	     return job.waitForCompletion(true)?0:1;
	}
	 
	public static void main(String[] args) throws Exception {
		
         int res = ToolRunner.run(new Configuration(), new ToSeqFile(), args);
	     System.exit(res);
	     
	}
	
}
