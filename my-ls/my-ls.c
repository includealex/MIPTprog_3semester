#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

extern int optind, opterr, errno;

struct statistics {
  unsigned char ind_all;
  unsigned char ind_long;
  unsigned char ind_inode;
  unsigned char ind_numidgid;
  unsigned char ind_recursive;
};

int print_dir(char* name, struct statistics inds);
int print_file(char* name, struct statistics inds);
int read_dir(DIR* dir, struct dirent** file);

int main(int argc, char* argv[]) {
  struct option inds[6] = {{.name = "all", .has_arg = 0, .flag = NULL, .val = 'a'},
                           {.name = "long", .has_arg = 0, .flag = NULL, .val = 'l'},
                           {.name = "inode", .has_arg = 0, .flag = NULL, .val = 'i'},
                           {.name = "numidgid", .has_arg = 0, .flag = NULL, .val = 'n'},
                           {.name = "recursive", .has_arg = 0, .flag = NULL, .val = 'R'},
                           {0}};

  struct statistics options = {0};

  int status;
  do {
    status = getopt_long(argc, argv, "alinR", inds, NULL);
    switch (status) {
      case 'a':
        options.ind_all = 1;
        break;
      case 'l':
        options.ind_long = 1;
        break;
      case 'i':
        options.ind_inode = 1;
        break;
      case 'n':
        options.ind_numidgid = 1;
        options.ind_long = 1;
        break;
      case 'R':
        options.ind_recursive = 1;
        break;
    }
  } while (status != -1);

  optind == argc ? print_dir("./", options) : print_dir(argv[optind], options);

  return 0;
}

int print_file(char* name, struct statistics inds) {
  struct stat buf;
  if (inds.ind_inode) {
    lstat(name, &buf);
    if (errno) {
      perror("lstat error");
      return -1;
    }
    printf("%lu ", buf.st_ino);
  }
  if (inds.ind_long) {
    lstat(name, &buf);
    if (errno) {
      perror("lstat error");
      return -1;
    }
    switch (S_IFMT & buf.st_mode) {
      case S_IFIFO:
        printf("p");
        break;
      case S_IFCHR:
        printf("c");
        break;
      case S_IFDIR:
        printf("d");
        break;
      case S_IFBLK:
        printf("b");
        break;
      case S_IFREG:
        printf("-");
        break;
      case S_IFLNK:
        printf("l");
        break;
      case S_IFSOCK:
        printf("s");
        break;
      default:
        printf("?");
    }

    for (int i = 8; i >= 0; i--) {
      if (buf.st_mode & (1 << i)) {
        switch ((i + 1) % 3) {
          case 0:
            printf("r");
            break;
          case 1:
            printf("x");
            break;
          case 2:
            printf("w");
            break;
        }
      } else
        printf("-");
    }

    printf(" %lu ", buf.st_nlink);

    if (!inds.ind_numidgid) {
      errno = 0;
      struct passwd* pass = getpwuid(buf.st_uid);
      if (!errno)
        printf("%s ", pass->pw_name);
      else
        printf("? ");
      errno = 0;
      struct group* gr = getgrgid(buf.st_gid);
      if (!errno)
        printf("%s ", gr->gr_name);
      else
        printf("? ");
    } else
      printf("%d %d ", buf.st_uid, buf.st_gid);

    printf("%6lu ", buf.st_size);

    char time_str[15] = {0};
    if (strftime(time_str, 15, "%b %d %H:%M", gmtime(&buf.st_mtime)) == 0)
      printf("?");
    else
      printf("%s ", time_str);

    printf("%s ", name);

  } else {
    printf("%s ", name);
  }
  return 0;
}

int print_dir(char* name, struct statistics inds) {
  int status = 0;
  DIR* cur_dir = opendir(name);
  struct dirent* file;
  if (cur_dir == NULL) {
    perror("uls (dir error)");
    return -1;
  }
  if (inds.ind_recursive)
    printf("%s:\n", name);

  while (1) {
    status = read_dir(cur_dir, &file);
    if (status != 0)
      break;
    if (file->d_name[0] != '.' || inds.ind_all) {
      print_file(file->d_name, inds);
      printf("\n");
    }
  }

  if (inds.ind_recursive) {
    printf("\n");
    rewinddir(cur_dir);
    while (1) {
      status = read_dir(cur_dir, &file);
      if (status != 0)
        break;
      if (file->d_type == DT_DIR) {
        if (!(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))) {
          if (!(file->d_name[0] == '.' && inds.ind_all)) {
            unsigned int name_len = strlen(name) + strlen(file->d_name) + 2;
            char full_dir[name_len];
            full_dir[0] = 0;
            strcat(full_dir, name);
            strcat(full_dir, file->d_name);
            full_dir[name_len - 2] = '/';
            full_dir[name_len - 1] = 0;
            print_dir(full_dir, inds);
          }
        }
      }
    }
  }
  return status;
}

int read_dir(DIR* dir, struct dirent** file) {
  errno = 0;
  *file = readdir(dir);
  if (*file == NULL) {
    if (errno != 0) {
      perror("file error");
      return -1;
    }

    return 1;
  }
  return 0;
}