#include <stdio.h>
#include <string.h>
//---------------------------------------------------------------------------------------

void make_print(int argc, char *argv[]);

//---------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("\n");
    return 0;
  }

  if (!strcmp(argv[1], "-n")) {
    if (argc == 2)
	  return 0;
    else {
      make_print(argc - 1, &argv[1]);  //not to print '-n'
      return 0;
    }
  }

  /*
  if (!strcmp(argv[1], "-e")) {
    if (argc > 2) {
      make_print(argc - 1, &argv[1]);  //not to print '-e'
    }

    printf("\n");
    return 0;
  }*/

  make_print(argc, argv);
  printf("\n");

  return 0;
}


//---------------------------------------------------------------------------------------

void make_print(int argc, char *argv[]) {
  int i;
  for (i = 1; i < argc - 1; ++i) 
    printf("%s ", argv[i]);
  printf("%s", argv[i]);
}
