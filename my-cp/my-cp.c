#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSZ 4096

int fdcp(int src, int dest);
int f_read(int src, char* buf, int size);
int f_write(int dst, char* buf, int size);
int filecope(char* src, char* dst, int ver, int force, int interactive);

extern int optind, errno;

int main(int argc, char* argv[]) {
  int ver = 0;
  int force = 0;
  int interactive = 0;
  int status = 0;

  struct option opts[4] = {
      {"ver", 0, NULL, 'v'}, {"force", 0, NULL, 'f'}, {"interactive", 0, NULL, 'i'}, {0}};

  while (status != -1) {
    status = getopt_long(argc, argv, "vfi", opts, NULL);
    switch (status) {
      case 'v':
        ver = 1;
        break;
      case 'f':
        force = 1;
        break;
      case 'i':
        interactive = 1;
        break;
    }
  }

  if ((argc - optind) > 2) {
    struct stat dirstat;
    if ((stat(argv[argc - 1], &dirstat) != -1) && (S_ISDIR(dirstat.st_mode) == 1))
      for (int i = optind; i < argc - 1; ++i) {
        filecope(argv[i], argv[argc - 1], ver, force, interactive);
      }
    else {
      if (errno) {
        perror("stat error");
        return -1;
      }
      printf("Last arg is not a directory\n");
      return -1;
    }
  } else if ((argc - optind) == 2) {
    return filecope(argv[optind], argv[optind + 1], ver, force, interactive);
  } else {
    printf("missing file operand\n");
    return -1;
  }

  return 0;
}

int filecope(char* src, char* dst, int ver, int force, int interactive) {
  struct stat filestat;
  char* fulldst = NULL;

  if ((stat(dst, &filestat) != -1) && (S_ISDIR(filestat.st_mode) == 1)) {
    fulldst = (char*)malloc(strlen(dst) + strlen(src) + 2);

    strcat(fulldst, dst);
    strcat(fulldst, "/");
    strcat(fulldst, src);
  } else {
    fulldst = (char*)malloc(strlen(dst) + 1);

    strcat(fulldst, dst);
  }

  if (stat(fulldst, &filestat) == 0) {
    if (interactive) {
      char conf;
      printf("overwrite '%s'? ", fulldst);
      scanf("%c", &conf);
      if ((conf == 'n') || (conf == 'N')) {
        return 1;
      }
    }

    if (force) {
      int fd = open(fulldst, O_RDWR | O_APPEND);
      if (fd == -1) {
        if (unlink(fulldst) == -1) {
          perror("unlink error");
          return -1;
        }
      }
      close(fd);
    }
  }

  if (ver)
    printf("%s -> %s\n", src, fulldst);

  int dstfd = open(fulldst, O_WRONLY | O_CREAT, 0664);
  if (dstfd == -1) {
    perror("destination file open error");
    return -1;
  }

  int srcfd = open(src, O_RDONLY);
  if (srcfd == -1) {
    perror("source file open error");
    return -1;
  }

  int status = fdcp(srcfd, dstfd);
  close(srcfd);
  close(dstfd);

  return status;
}

int f_read(int src, char* buf, int size) {
  int read_bytes;

  read_bytes = read(src, buf, size);

  if (read_bytes == -1) {
    perror("Read error");
  }

  return read_bytes;
}

int f_write(int dst, char* buf, int size) {
  int write_bytes = 0;
  int offset = 0;

  while (offset < size) {
    write_bytes = write(dst, buf + offset, size);

    if (write_bytes == -1) {
      perror("Write error");
      return -1;
    }

    offset += write_bytes;
  }

  return 0;
}

int fdcp(int src, int dst) {
  int read_err = 0;
  int write_err = 0;

  char buf[BUFSZ];

  read_err = f_read(src, buf, BUFSZ);

  while (read_err != 0) {
    if (read_err == -1) {
      return -1;
    }

    write_err = f_write(dst, buf, read_err);

    if (write_err == -1) {
      return -1;
    }

    read_err = f_read(src, buf, BUFSZ);
  }

  return 0;
}