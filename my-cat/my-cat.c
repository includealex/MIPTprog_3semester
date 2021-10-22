#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//---------------------------------------------------------------------------------

enum Params {BUFFSIZE = 1024,};

//---------------------------------------------------------------------------------

int consoleWork (int fd);
int FilesWork (int fd, int argc, char* argv[]);

//---------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  int fd = 0;

  if (argc == 1) {
    return consoleWork(0);
  }

  else {
    return FilesWork(fd, argc, argv);
  }
}

//---------------------------------------------------------------------------------

int consoleWork (int fd) {
  char arr[BUFFSIZE];
  int fl_read = 0;  //full read
  int fl_write = 0; //full write
  int pt_write = 0; //part write

  while(1) {

    // 1. Reading

    fl_read = read(fd, arr, BUFFSIZE);

    if(fl_read < 0) {
      perror("Read error in function consoleWork\n");
      return errno;
    }

    if(!fl_read)
      break;

    // 2. Writing

    fl_write = 0;

    while(fl_write < fl_read) {
      pt_write = write(1, arr + fl_write, fl_read - fl_write);

      if(pt_write < 0) {
        perror("Write error in function consoleWork\n");
        return errno;
      }

      fl_write += pt_write;
    }
  }
 
  return 0;
}

//---------------------------------------------------------------------------------

int FilesWork (int fd, int argc, char* argv[]) {
  int i = 0;

  for (i = 1; i < argc; ++i) {
    fd = open(argv[i], O_RDONLY);

    if(fd < 0) {
      perror("Open error in function FilesWork\n");
      return errno;
    }

    if(consoleWork(fd) < 0) {
      perror("Error in function consoleWork\n");
      return errno;
    }

    if(close(fd) < 0) {
      perror("Closing error in function FilesWork\n");
      return errno;
    }
  }

  return 0;
}
