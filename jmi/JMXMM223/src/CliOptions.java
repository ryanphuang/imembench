import org.kohsuke.args4j.Option;

public class CliOptions {
  @Option(name="-p", usage="Local JMI port to connect to")
  public int port = -1;

  @Option(name="-t",usage="Time period (sec) to get the mm usage")
  public long period = 2;
}
