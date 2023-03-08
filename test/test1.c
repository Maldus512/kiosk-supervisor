#include <stdio.h>

int main(int argc, char **argv) {

  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'v') {
    printf("v1");
    return 0;
  }

  printf("avviato %d %s\n", argc, argv[1]);
  for (;;) {
  }

  return 0;
}
