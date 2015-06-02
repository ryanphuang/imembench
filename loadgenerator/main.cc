#include "imembench.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
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
  return 0;
}
