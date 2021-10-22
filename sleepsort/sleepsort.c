#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
  for(int i = 1; i < argc; ++i) {
    int pr = fork();

    if(pr == 0) {
      usleep(1000 * atoi(argv[i]));
      printf("%s \n", argv[i]);
      return 0;
    }
  }

  return 0;
}
