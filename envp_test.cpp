#include <stdio.h>
#include <stdlib.h>

extern char ** environ;

int main(int argc, char ** argv) {
  if (argc == 0 or argv == NULL) {
  }

  int status = setenv("PATH_1u92u193", "test_val", 1);
  if (status == -1) {
    printf("error");
  }
  char ** env = environ;
  while (*env != NULL) {
    char * thisEnv = *env;
    printf("%s\n", thisEnv);
    env++;
  }
  // char * res = getenv("PATH12");
  // printf("%s\n", res);

  return 0;
}
