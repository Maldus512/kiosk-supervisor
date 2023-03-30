#include "controller/controller.h"
#include "log.h"
#include "lvgl.h"
#include "model/model.h"
#include "utils/socketq.h"
#include "utils/system_time.h"
#include "view/view.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#if USE_EVDEV
#include "indev/evdev.h"
#endif
#if USE_FBDEV
#include "display/fbdev.h"
#endif
#if USE_SDL
#include "sdl/sdl.h"
#endif

int main(int argc, char **argv) {

  if (argc < 6) {
    log_error("Incorrect number of parameters (%d), it should be at least 5",
              argc);
    return -1;
  }

  static unsigned long last_invoked = 0;
  socketq_t fsocketq = {0};
  socketq_init(&fsocketq, "/tmp/fsocketq.sock", 1);
  socketq_t tsocketq = {0};
  socketq_init(&tsocketq, "/tmp/tsocketq.sock", 1);
  socketq_t msocketq = {0};
  socketq_init(&msocketq, "/tmp/msocketq.sock", 1);

  model_t model = {
      .sup_version = "v1",
      .app_version = "v0",
      .app_path = argv[1],
      .update_path = argv[2],
      .log_paths = argv + 6,
      .n_log_paths = argc - 6,
      .mount_path = argv[3],
      .fsocketq = &fsocketq,
      .tsocketq = &tsocketq,
      .msocketq = &msocketq,

      .period = atoi(argv[4]),
      .term_per_period = atoi(argv[5]),
      .pid = -1,
  };

  if (model.n_log_paths > 32) {
    printf("[x] The maximum number of logs is 32");
  }

  log_info("log totali %d", model.n_log_paths);

  lv_init();
#if USE_FBDEV
  fbdev_init();
#endif
#if USE_EDEV
  evdev_init();
#endif
#if USE_SDL
  sdl_init();
#endif

  model_init(&model);
  view_init(&model, controller_manage_message);
  controller_init(&model);

  printf("Begin main loop\n");
  for (;;) {
    // Run LVGL engine
    if (last_invoked > 0) {
      lv_tick_inc(get_millis() - last_invoked);
    }
    last_invoked = get_millis();
    lv_timer_handler();

    controller_manage(&model);

    usleep(1000);
  }

  return 0;
}
