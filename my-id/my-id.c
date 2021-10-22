#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

//-------------------------------------------------------------------------------------------

enum Params {size = 10000,};

//-------------------------------------------------------------------------------------------

void userInfo();
void exactInfo(char *exact_name);
//-------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {

  if (argc == 1)
    userInfo();

  else
    exactInfo(argv[1]);

  return 0;
}

//-------------------------------------------------------------------------------------------

void userInfo() {
  int i = 0;
  int s_gr = 0;           // size_getgroups
  struct passwd *pwd; 
  gid_t *gr;          // groups

  gr = calloc(size, sizeof(gid_t));
  pwd = getpwuid(getuid());
  s_gr = getgroups(size, gr);

  printf("uid=%d(%s) gid=%d(%s) groups=%d(%s)", getuid(), pwd->pw_name,
                                                getgid(), getgrgid(getgid())->gr_name,
                                                pwd->pw_gid, getgrgid(pwd->pw_gid)->gr_name); 

  for (i = 0; i < s_gr - 1; ++i) {
    printf(",%d(%s)", gr[i], getgrgid(gr[i])->gr_name);
  }

  printf("\n");

  free(gr);
}

void exactInfo(char *exact_name) {
  int eSize = 0;
  int i = 0;
  int es_gr = 0;
  struct passwd *ewd;
  gid_t *egr;

  ewd = getpwnam(exact_name);

  if(!ewd) {
    printf("id :'%s': no such user\n", exact_name);
  }


  else {
    egr = calloc(size, sizeof(gid_t));

    eSize = size;
    es_gr = getgrouplist (exact_name, ewd->pw_gid, egr, &eSize);

    printf("uid=%d(%s) gid=%d(%s) groups=%d(%s)", ewd->pw_uid, ewd->pw_name,
                                                ewd->pw_gid, getgrgid(ewd->pw_gid)->gr_name, 
                                                ewd->pw_gid, getgrgid(ewd->pw_gid)->gr_name);

    for (i = 1; i < eSize; ++i) {
      printf(",%d(%s)", egr[i], getgrgid(egr[i])->gr_name);
    }
    printf("\n");

    free(egr);
  }
}
