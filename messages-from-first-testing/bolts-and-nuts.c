#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MSGSZ 1

extern int errno;

struct msgbuf_imitator {
  long mtype;
  char mtext[MSGSZ];
};

void create_plan(int trinketInit, int bolt, int nut) {
  struct msgbuf_imitator msgInit = {1, {0}};

  msgsnd(trinketInit, &msgInit, MSGSZ, 0);

  struct msgbuf_imitator msgInitWorker = {1, {1}};

  msgsnd(nut, &msgInitWorker, MSGSZ, 0);
  msgsnd(nut, &msgInitWorker, MSGSZ, 0);
  msgsnd(bolt, &msgInitWorker, MSGSZ, 0);
}

int nutter(int trinketInit, int bolt, int nut, int workPlan) {
  for (;;) {
    int val = 0;
    struct msgbuf_imitator msg;

    msgrcv(nut, &msg, MSGSZ, 1, 0);
    msgrcv(workPlan, &msg, MSGSZ, 1, 0);

    val = msg.mtext[0];
    msgsnd(workPlan, &msg, MSGSZ, 0);

    if (!val) {
      return 0;
    }

    printf("nut [%d] in work\n", getpid());

    msgrcv(trinketInit, &msg, MSGSZ, 1, 0);
    val = msg.mtext[0];

    if (val == 2) {
      msgrcv(workPlan, &msg, MSGSZ, 1, 0);

      val = --msg.mtext[0];
      msgsnd(workPlan, &msg, MSGSZ, 0);

      printf("nut [%d] cought a detail\n", getpid());
      printf("nut [%d] is done\n\n", getpid());

      create_plan(trinketInit, bolt, nut);
    } else {
      struct msgbuf_imitator msgForSend = {1, {val + 1}};

      msgsnd(trinketInit, &msgForSend, 1, 0);
      printf("nut [%d] is done\n", getpid());
    }
  }
}

int boltter(int trinketInit, int bolt, int nut, int workPlan) {
  for (;;) {
    int val = 0;
    struct msgbuf_imitator msg;

    msgrcv(bolt, &msg, MSGSZ, 1, 0);
    msgrcv(workPlan, &msg, MSGSZ, 1, 0);

    val = msg.mtext[0];
    msgsnd(workPlan, &msg, MSGSZ, 0);

    if (!val) {
      return 0;
    }

    printf("bolt [%d] in work\n", getpid());

    msgrcv(trinketInit, &msg, MSGSZ, 1, 0);
    val = msg.mtext[0];

    if (val == 2) {
      msgrcv(workPlan, &msg, MSGSZ, 1, 0);

      --msg.mtext[0];
      msgsnd(workPlan, &msg, MSGSZ, 0);

      printf("bolt [%d] cought a detail\n", getpid());
      printf("bolt [%d] is done\n\n", getpid());

      create_plan(trinketInit, bolt, nut);
    } else {
      struct msgbuf_imitator msgForSend = {1, {val + 1}};

      msgsnd(trinketInit, &msgForSend, MSGSZ, 0);
      printf("bolt [%d] is done\n", getpid());
    }
  }
}

int main(int argc, char* argv[]) {
  int ntrinkets = 1e6;

  if (argc == 2) {
    ntrinkets = atoi(argv[1]);
  }

  if (argc > 2) {
    printf("Incorrect number of arguments\n");
    return 0;
  }

  int workPlan = msgget(IPC_PRIVATE, IPC_CREAT | 0660);
  struct msgbuf_imitator msgInit = {1, {ntrinkets}};

  msgsnd(workPlan, &msgInit, MSGSZ, 0);

  int trinketInit = msgget(IPC_PRIVATE, IPC_CREAT | 0660);
  int bolt = msgget(IPC_PRIVATE, IPC_CREAT | 0660);
  int nut = msgget(IPC_PRIVATE, IPC_CREAT | 0660);

  create_plan(trinketInit, bolt, nut);

  int pid = fork();
  if (!pid) {
    nutter(trinketInit, bolt, nut, workPlan);
    return 0;
  }

  pid = fork();
  if (!pid) {
    nutter(trinketInit, bolt, nut, workPlan);
    return 0;
  }

  pid = fork();
  if (!pid) {
    boltter(trinketInit, bolt, nut, workPlan);
    return 0;
  }

  for (int i = 0; i < 3; ++i) {
    wait(NULL);
  }

  msgctl(trinketInit, IPC_RMID, NULL);
  msgctl(bolt, IPC_RMID, NULL);
  msgctl(nut, IPC_RMID, NULL);
  msgctl(workPlan, IPC_RMID, NULL);

  printf("!!!No way, it really works!!!\n");
  return 0;
}