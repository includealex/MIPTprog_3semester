#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern int errno;

void captain(int ride_num, int sem_id, int seats_num) {
  struct sembuf lifting_voyagers[2] = {{0, seats_num, 0}, {3, -1, 0}};
  struct sembuf wait_lifting_voyagers = {0, 0, 0};
  struct sembuf ride[2] = {{2, -1, 0}, {3, 1, 0}};
  struct sembuf wait_off[2] = {{0, -seats_num, 0}, {2, 1, 0}};

  printf("Captain init\n");

  for (int i = 0; i < ride_num; i++) {
    printf("\n\n");

    printf("Captain: Accepting voyagers\n");
    semop(sem_id, lifting_voyagers, 2);
    semop(sem_id, &wait_lifting_voyagers, 1);
    printf("Captain: All voyagers seated\n");
    semop(sem_id, ride, 2);
    printf("Captain: ride ended\n");
    semop(sem_id, wait_off, 2);
    printf("Captain: all off\n");
    sleep(3);
  }
}

int voyagers(int sem_id, int pass_num) {
  struct sembuf come_up[3] = {{0, -1, 0}, {1, -1, 0}, {3, 0, 0}};
  struct sembuf come_down[3] = {{0, 1, 0}, {1, -1, 0}, {2, 0, 0}};
  struct sembuf trup_is_free = {1, 1, 0};

  for (int i = 0; i < pass_num; ++i) {
    int id = fork();

    if (id) {
      printf(" Voyager registered\n");
      for (;;) {
        if ((semop(sem_id, come_up, 3) == -1) && ((errno == EIDRM) || (errno == EINVAL))) {
          return 0;
        }

        semop(sem_id, &trup_is_free, 1);
        sleep(1);
        printf("Voyagers got on the yacht\n");
        semop(sem_id, come_down, 3);
        semop(sem_id, &trup_is_free, 1);
        sleep(1);
        printf("Voyagers exited the yacht\n");
      }
    }
  }
}

int main(int argc, char* argv[]) {
  int sem_id = semget(IPC_PRIVATE, 4, 0660);
  unsigned short init_array[4] = {0, 1, 1, 1};

  semctl(sem_id, 0, SETALL, init_array);

  int pass_num = atoi(argv[1]);
  int ride_num = atoi(argv[2]);
  int seats_num = atoi(argv[3]);

  if (pass_num < seats_num) {
    seats_num = pass_num;
  }

  int res_voyagers = voyagers(sem_id, pass_num);
  if (res_voyagers == 0) {
    return 0;
  }

  captain(ride_num, sem_id, seats_num);

  semctl(sem_id, IPC_RMID, 0);

  printf("!!!Hooray!!!End of trips!!!\n");

  return 0;
}