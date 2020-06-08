#include "runner.hpp"
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

int main(int argc, char**argv) {
  //rankFeatures();
  //evalNormalizeRigid();
  //evalTasks();
  //bruteSubmission();
  //bruteSolve();
  int only_sid = -1;
  if (argc >= 2) {
    only_sid = atoi(argv[1]);
    printf("Running only task # %d\n", only_sid);
  }
  int maxdepth = -1;
  if (argc >= 3) {
    maxdepth = atoi(argv[2]);
    printf("Using max depth %d\n", maxdepth);
  }
  run(only_sid, maxdepth);
}
