#include "arguments.h"
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void print_help(const char *path);

arguments_t arguments_parse(int argc, char **argv) {
  arguments_t arguments = {
      .app_path = "",
      .mount_path = "",
      .update_path = "",
  };

  opterr = 0;

  struct option long_options[] = {
      {"update-path", required_argument, 0, 'u'},
      {"mount-path", required_argument, 0, 'm'},
      {"logs", required_argument, 0, 'l'},
      {"period", required_argument, 0, 'p'},
      {"terminations-per-period", required_argument, 0, 't'},
      {"black-screen-timer", required_argument, 0, 'b'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0},
  };

  while (optind < argc) {
    int option_index = 0;

    int c =
        getopt_long(argc, argv, ":u:m:l:p:t:b:h", long_options, &option_index);

    switch (c) {
    case 'h':
      print_help(argv[0]);
      break;

    case 'u':
      arguments.update_path = optarg;
      break;

    case 'm':
      arguments.mount_path = optarg;
      break;

    case 'l':
      arguments.log_paths = realloc(
          arguments.log_paths, (arguments.n_log_paths + 1) * sizeof(char *));
      assert(arguments.log_paths != NULL);
      arguments.log_paths[arguments.n_log_paths] = optarg;
      arguments.n_log_paths++;
      break;

    case 'p':
      arguments.period = atoi(optarg);
      break;

    case 't':
      arguments.term_per_period = atoi(optarg);
      break;

    case 'b':
      arguments.black_screen_timer = atoi(optarg);
      break;

    case -1:
      // Positional arguments
      arguments.app_path = argv[optind];
      optind++;
      break;

    case ':':
      printf("Missing argument for option %i!\n", optind + 1);
      print_help(argv[0]);
      break;

    case '?':
      printf("Found unknown option %i!\n", optind + 1);
      print_help(argv[0]);
      break;
    }
  }

  return arguments;
}

static void print_help(const char *path) {
  printf("Usage:\n"
         "\t%s [options] <app path>\n\n"
         "Options:\n"
         "\t-h,--help\t\t\tPrint this help and quit\n"
         "\t-u,--update-path\t\t\t\n"
         "\t-m,--mount-path\t\t\t\n"
         "\t-l,--logs\t\t\t\n"
         "\t-p,--period\t\t\t\n"
         "\t-t,--terminations-per-period\t\t\t\n"
         "\t-b,--black-screen-timer\t\t\t\n",
         path);

  exit(0);
}
