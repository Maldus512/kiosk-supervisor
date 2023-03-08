#include <stdio.h>

int main(int argc, char **argv) {

  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'v') {
    printf("v1.updated");
    return 0;
  }

  printf("avviato update\n");
  for (;;) {
  }

  return 0;
}
