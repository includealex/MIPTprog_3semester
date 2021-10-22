#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  int pr = fork();
  struct timeval begTime;

  if(gettimeofday (&begTime, NULL)) {
    perror("Time crash -_-\n");
    return -1;
  }

  if (pr == 0) {
    execvp(argv[1], argv + 1);
    perror("Execution failure\n");
    return -1;
  }

  wait(NULL);

  struct timeval endTime;

  if(gettimeofday (&endTime, NULL)) {
    perror("Time crash -_-\n");
    return -1;
  }

  double time_ex = (double)(endTime.tv_sec - begTime.tv_sec) + (double)(endTime.tv_usec - begTime.tv_usec) / 1e+6;
  printf("%lf\n", time_ex);


  return 0;
}
