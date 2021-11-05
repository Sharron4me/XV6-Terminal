#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  int pid = atoi(argv[1]);
  printf(1,"%d\n",pid);
  printf(1,"Number of files opened: %d\n",numOpenFiles(pid));
  printf(1,"Memory allocated: %d\n",memAlloc(pid));
  getprocesstimedetails(pid);
  exit(0);
}
