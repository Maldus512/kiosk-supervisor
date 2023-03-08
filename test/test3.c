#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {

  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'v') {
    printf("v3");
    return 0;
  }

  printf("killing me\n");

  return -1;
}
