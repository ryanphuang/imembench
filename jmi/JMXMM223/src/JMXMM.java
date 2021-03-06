import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.lang.Thread;
import java.net.InetAddress;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import javax.management.ObjectName;
import javax.management.openmbean.CompositeData;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import org.kohsuke.args4j.CmdLineException;
import org.kohsuke.args4j.CmdLineParser;

//import org.slf4j.Logger;
//import org.slf4j.LoggerFactory;

public class JMXMM {
  private static final String URL_TEMPLATE = "service:jmx:rmi:///jndi/rmi://localhost:PORT/jmxrmi";
  
  private JMXServiceURL JMXUrl = null;
  private long period = -1;
  private String logfile = null;
  
  public JMXMM(int port, long period, String file) {
    System.setProperty("java.rmi.server.hostname", "localhost");
    this.JMXUrl = getJMXUrl(Integer.toString(port));
    this.period = period;
    this.logfile = file;
    
    if(JMXUrl == null) {
      //error is printed out in getJMXUrl
      System.exit(-1);
    } 
    
    try {
      JMXConnectorFactory.connect(JMXUrl, null);
    } catch (Exception e) {
      e.printStackTrace();
      System.exit(-1);
    }
  }
  
  public static void getContinousMemoryUsages(JMXServiceURL url, long period, String logfile) {
    try {
      @SuppressWarnings("resource")
      PrintWriter pw = new PrintWriter(new FileWriter(logfile));
      pw.println("INIT USED COMMITTED MAX");
      
      while(true) {
        try {
          JMXConnector jmxc = JMXConnectorFactory.connect(url, null);
          Object o = jmxc.getMBeanServerConnection().getAttribute(new ObjectName("java.lang:type=Memory"), "HeapMemoryUsage");
          CompositeData cd = (CompositeData) o;
          
          pw.println(cd.get("init") + " " + cd.get("used") + " " + cd.get("committed") + " " + cd.get("max"));
          pw.flush();
          
          Thread.sleep(period);
          
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
      
    } catch(Exception e) {
      e.printStackTrace();
    }
  }
  
  private static JMXServiceURL getJMXUrl(String pstr) {
    try {
      return new JMXServiceURL(URL_TEMPLATE.replace("PORT", pstr));
    } catch(Exception e) {
      e.printStackTrace();
      return null;
    }
  }
  
  public void run() {
    new Thread() {
      public void run() {
        JMXMM.getContinousMemoryUsages(JMXUrl, period, logfile);
      }
    }.start();
  }
  
  public static void main(String[] args) throws IOException {
    CliOptions options = new CliOptions();
    CmdLineParser parser = new CmdLineParser(options);
    try {
      parser.parseArgument(args);
    } catch(CmdLineException e) {
      System.err.println(e.getMessage());
      System.err.println("java -jar jmxmm.jar [options...] arguments...");
      parser.printUsage(System.err);
      return;
    }
    
    if(options.port <= 0 || options.port > 65535) {
      System.err.println("[ERROR] port (-p) should be an positive integer int the range of [1, 65535]");
      return;
    }
    
    if(options.logfile.length() == 0) {
      SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
      options.logfile = "mmlog." + InetAddress.getLocalHost().getHostName() + "." + sdf.format(Calendar.getInstance().getTime());
    }
    
    String logpath = (options.logdir.length() == 0) ? options.logfile : options.logdir + "/" + options.logfile;  
    
    JMXMM jmxmm = new JMXMM(options.port, options.period*1000, logpath);
    jmxmm.run();
  }
}
