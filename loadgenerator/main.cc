#include "imembench.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

void testConfigs()
{
  BenchConfigParser parser;
  parser.parse("test.ini");
  BenchConfigSections sections;
  parser.getSections(sections);
  BenchConfigSections::iterator sit;
  for (sit = sections.begin(); sit != sections.end(); ++sit) {
    printf("section: %s\n", sit->c_str());
    BenchConfigMap configs = parser.getSectionConfigs(sit->c_str());
    BenchConfigMap::iterator cit;
    for (cit = configs.begin(); cit != configs.end(); ++cit) {
      printf("%s=%s\n", cit->first.c_str(), cit->second.c_str());
    }
  }
}

int main()
{
  /*
  ConnectionConfig rcconf("localhost", 12246, "fast+udp", NULL, "test");
  printf("%s\n", rcconf.getLocator());
  RamCloudDriver rcdriver;
  bool ok;
  ok = rcdriver.init(&rcconf);
  if (!ok) {
    printf("fail to initialize ramcloud driver\n");
    exit(1);
  }
  char buf[256];
  rcdriver.write("fooo", "bar", 4); // also store the '\0' in value
  rcdriver.read("fooo", buf, sizeof(buf));
  printf("['%s']=%s\n", "fooo", buf);
  */
  testConfigs();
  return 0;
}
