#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const int NBOGATYRES = 33;

struct msgbuf_imitator {
  long mtype;
  char mtext[1];
};

void fillwithlit(int queue_id, char* lit) {
  int len = strlen(lit);
  struct msgbuf_imitator cur = {1, 0};

  for (int i = 0; i < NBOGATYRES; ++i) {
    if (i >= len) {
      int poison = -666;
      cur.mtext[0] = poison;
    }
    else{
      cur.mtext[0] = lit[i];
    }

    msgsnd(queue_id, &cur, 1, 0);
  }
}

char* discoverLiterals(char* ballad) {
  char allSym[1024] = {};
  int len = strlen(ballad);
  int litSize = 0;

  for (int i = 0; i < len; ++i) {
    if (!allSym[ballad[i]]) {
      allSym[ballad[i]] = 1;
      ++litSize;
    }
  }

  char* literals = (char*)calloc(litSize + 1, sizeof(char));

  int place = 0;
  for (int i = 0; i < litSize; ++i) {
    while (place < 1024 && !allSym[place++]) {
      ;
    }

    literals[i] = place - 1;
  }
  literals[litSize] = 0;

  return literals;
}

char* chronicle(int argc, char** argv) {
  int sumLen = 0;
  for (int i = 1; i < argc; ++i) {
    sumLen += strlen(argv[i]) + 1;
  }

  char* ballad = (char*)calloc((sumLen + 1), sizeof(char));
  ballad[0] = 0;

  for (int i = 1; i < argc; ++i) {
    strcat(ballad, argv[i]);
    strcat(ballad, " ");
  }

  ballad[sumLen - 1] = 0;

  return ballad;
}

int ballader(char* ballad) {
  key_t key;
  key = ftok(".", 'a');

  if (key == -1) {
    printf("Ftok error\n");
    return 0;
  }

  int literalsMsg = msgget(key, 0660);
  key = ftok(".", 'b');

  if (key == -1) {
    printf("Ftok error\n");
    return 0;
  }

  int sem_id = semget(key, 1, 0660);

  struct msgbuf_imitator forRcv;
  msgrcv(literalsMsg, &forRcv, 1, 1, 0);

  if (forRcv.mtext[0] == -1) {
    return 0;
  }

  char mySym = forRcv.mtext[0];
  int len = strlen(ballad);
  int countMySym = 0;

  for (int i = 0; i < len; ++i) {
    if (ballad[i] == mySym) {
      ++countMySym;
    }
  }

  struct sembuf nxt = {0, 1, 0};

  for (int i = 0; i < countMySym; ++i) {
    while (ballad[semctl(sem_id, 0, GETVAL)] != mySym) {
      ;
    }

    putchar(mySym);
    printf(" [%d]\n", getpid());
    fflush(NULL);

    semop(sem_id, &nxt, 1);
  }

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Invalid number of arguments\n");
    return 0;
  }

  char* ballad = chronicle(argc, argv);
  char* literals = discoverLiterals(ballad);

  key_t key = ftok(".", 'a');
  if (key == -1) {
    printf("Ftok error\n");
    return 0;
  }

  int literalsMsg = msgget(key, IPC_CREAT | 0666);

  fillwithlit(literalsMsg, literals);
  free(literals);

  key = ftok(".", 'b');
  if (key == -1) {
    printf("Ftok error\n");
    return 0;
  }

  int sem_id = semget(key, 1, IPC_CREAT | IPC_EXCL | 0660);
  semctl(sem_id, 0, SETVAL, 0);

  pid_t pid = 0;

  for (int i = 0; i < NBOGATYRES; ++i) {
    pid = fork();

    if (!pid) {
      ballader(ballad);
      free(ballad);

      return 0;
    }
  }

  for (int i = 0; i < NBOGATYRES; ++i) {
    wait(NULL);
  }

  free(ballad);
  msgctl(literalsMsg, IPC_RMID, NULL);
  semctl(sem_id, IPC_RMID, 0);

  return 0;
}