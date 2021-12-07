#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern int errno;

struct msgbuf_imitator {
    long mtype;
    char mtext[1];
};

int create_plan(int pl_id){
    errno = 0;

    struct msgbuf_imitator nut_msg = {1, {1}};
    struct msgbuf_imitator bolt_msg = {2, {1}};
    struct msgbuf_imitator not_done_msg = {3, {0}};
    struct msgbuf_imitator done_msg = {3, {1}};

    msgsnd(pl_id, &nut_msg, 1, 0);
    msgsnd(pl_id, &nut_msg, 1, 0);
    msgsnd(pl_id, &bolt_msg, 1, 0);
    msgsnd(pl_id, &not_done_msg, 1, 0);
    msgsnd(pl_id, &not_done_msg, 1, 0);
    msgsnd(pl_id, &done_msg, 1, 0);
        if (errno != 0) {
            return -1;
    }

    return 0;
}

int boltter(int pl_id){
    struct msgbuf_imitator msg;
    struct msgbuf_imitator nuts_done = {1, {0}};
    int bolt_init = 0;
    printf("Putted bolts\n");
    for(;;) {
        if (!bolt_init) {
            printf("Got a bolt\n");
            ++bolt_init;
        }
        msgrcv(pl_id, &msg, 1, 2, 0);
        if (*msg.mtext) {
            printf("Added a bolt\n");
            --bolt_init;
            msgrcv(pl_id, &msg, 1, 3, 0);
            if (*msg.mtext) {
                printf("Swapped the part\n");
                msgsnd(pl_id, &nuts_done, 1, 0);
                msgsnd(pl_id, &nuts_done, 1, 0);
                create_plan(pl_id);
            }
        }
        if (errno != 0) {
            return -1;
        }
    }
}

int nutter(int pl_id){
    struct msgbuf_imitator msg;
    struct msgbuf_imitator bolts_done = {2, {0}};
    struct msgbuf_imitator nuts_done = {1, {0}};
    int nut_init = 0;
    printf("Putting_nuts\n");
    for(;;) {
        if (!nut_init) {
            printf("Received a nut\n");
            ++nut_init;
        }
        msgrcv(pl_id, &msg, 1, 1, 0);
        if (*msg.mtext) {
            printf("Putted a nut\n");
            --nut_init;
            msgrcv(pl_id, &msg, 1, 3, 0);
            if (*msg.mtext) {
                msgsnd(pl_id, &bolts_done, 1, 0);
                msgsnd(pl_id, &nuts_done, 1, 0);
                create_plan(pl_id);
            }
        }
    }
        if (errno != 0) { 
            return -1;
    }
}

int main() {
    int pl_id = msgget(IPC_PRIVATE, IPC_CREAT | 0660);

    create_plan(pl_id);

    int pid = fork();
    if (!pid) {
        nutter(pl_id);
        return 0;
    }
    pid = fork();
    if (!pid) {
        nutter(pl_id);
        return 0;
    }
    pid = fork();
    if (!pid) {
        boltter(pl_id);
        return 0;
    }

    usleep(1);

    printf("Everything's done. We have created an art-looking like detail!!!\n");

    msgctl(pl_id, IPC_RMID, NULL);
    for(int i = 0; i < 3; i++){
        wait(NULL);
    }

    return 0;
}