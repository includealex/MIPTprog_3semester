#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
  int i = 0;
  int num = 0;
  pid_t pid = 0;

  num = *argv[1] - '0';

 while(num != 0) {
    pid = fork();
    int st;

    if (pid == 0) {
      printf("%x %x\n", getpid(), getppid());
      --num;
    }


    else if(pid != 0) {
      wait(NULL);
      return 0;
    }

  } 



  return 0;
}
