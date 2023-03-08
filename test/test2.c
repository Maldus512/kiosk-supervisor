#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {

  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'v') {
    printf("v2");
    return 0;
  }

  printf("avviato\n");
  usleep(500 * 1000);
  printf("delay\n");

  return 0;
}
