#ifndef ARGUMENTS_H_INCLUDED
#define ARGUMENTS_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  char *app_path;
  char *update_path;
  char *mount_path;

  size_t n_log_paths;
  char **log_paths;

  uint32_t period;
  uint16_t term_per_period;

  uint32_t black_screen_timer;
} arguments_t;

arguments_t arguments_parse(int argc, char **argv);

#endif
