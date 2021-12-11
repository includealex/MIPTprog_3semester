#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LIMIT 1024

void run_cases(FILE* stdin) {
  do {
    size_t iter;
    int pass_pipe = -1;
    int pid;
    char* bufstrtr = (char*)malloc(LIMIT);
    char* bufstr[LIMIT];

    printf("Cases attractive shell > ");

    fgets(bufstrtr, LIMIT, stdin);

    bufstrtr[strlen(bufstrtr) - 1] = 0;

    if (!strcmp(bufstrtr, "exit")) {
      break;
    }

    bufstr[0] = strtok(bufstrtr, "|");
    if (bufstr[0] == NULL)
      continue;

    for (iter = 0; bufstr[iter] != NULL; ++iter) {
      bufstr[iter + 1] = strtok(NULL, "|");
    }

    for (int i = 0; i < iter; ++i) {
      int pipefd[2];

      if (pipe(pipefd) == -1) {
        printf("Lost in pipe\n");
        break;
      }

      pid = fork();
      if (pid == -1) {
        printf("Lost in fork\n");
        break;
      }

      if (pid != 0) {
        close(pipefd[1]);
        close(pass_pipe);
        pass_pipe = pipefd[0];
      } else {
        if (pass_pipe != -1) {
          if (dup2(pass_pipe, 0) == -1) {
            printf("Lost in dup\n");
            exit(-1);
          }
        }

        if (i < (iter - 1)) {
          if (dup2(pipefd[1], 1) == -1) {
            printf("Lost in dup\n");
            exit(-1);
          }
        }

        char** exec_args = bufstr + i;
        int argc_num;

        exec_args[0] = strtok(bufstr[i], " ");
        for (argc_num = 0; exec_args[argc_num] != NULL; ++argc_num) {
          exec_args[argc_num + 1] = strtok(NULL, " ");
        }

        execvp(exec_args[0], exec_args);
        perror(exec_args[0]);

        close(pipefd[0]);
        close(pipefd[1]);
        close(pass_pipe);
        exit(-1);
      }
    }

    waitpid(pid, NULL, WUNTRACED);
    close(pass_pipe);

  } while (!feof(stdin));
}

int main(int argc, char* argv[]) {
  run_cases(stdin);

  return 0;
}