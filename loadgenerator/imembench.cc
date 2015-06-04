#include "imembench.h"
#include <stdio.h>

void readIntensive(BenchDriverBase *driver);
void writeOnly(BenchDriverBase *driver);

BenchMark gBenchmarks[] = {
  {"readIntensive", readIntensive},
  {"writeOnly", writeOnly},
};

size_t gNbench = sizeof(gBenchmarks) / sizeof(gBenchmarks[0]);



void readIntensive(BenchDriverBase *driver)
{

};

void writeOnly(BenchDriverBase *driver)
{


};

void runBenchMarks()
{
  size_t i, j;
  for (i = 0; i < gNdriver; ++i) {
    BenchDriverBase *driver = gDrivers[i];
    for (j = 0; j < gNbench; ++j) {
      BenchMark bench = gBenchmarks[j];
      printf("running benchmark '%s' on '%s'\n", bench.name, driver->getName());
    }
  }
}
