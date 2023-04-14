#include "controller/controller.h"
#include "log.h"
#include "lvgl.h"
#include "model/model.h"
#include "utils/arguments.h"
#include "utils/socketq.h"
#include "utils/system_time.h"
#include "view/view.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#if USE_EVDEV
#include "display/fbdev.h"
#include "indev/evdev.h"
#endif
#if USE_SDL
#include "sdl/sdl.h"
#endif

int main(int argc, char **argv) {

  arguments_t arguments = arguments_parse(argc, argv);
  if (arguments.app_path == NULL) {
    printf("You must specify an application to supervise!\n");
    exit(1);
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
      .app_path = arguments.app_path,
      .update_path = arguments.update_path,
      .log_paths = arguments.log_paths,
      .n_log_paths = arguments.n_log_paths,
      .mount_path = arguments.mount_path,
      .fsocketq = &fsocketq,
      .tsocketq = &tsocketq,
      .msocketq = &msocketq,

      .period = arguments.period,
      .term_per_period = arguments.term_per_period,
      .black_screen_timer = arguments.black_screen_timer,
      .pid = -1,
  };

  if (model.n_log_paths > 32) {
    printf("[x] The maximum number of logs is 32");
  }

  for (int i = 0; i < model.n_log_paths; i++) {
    printf("Logs %s\n", model.log_paths[i]);
  }

  lv_init();
#if USE_FBDEV
  evdev_init();
  fbdev_init();
  view_init(&model, controller_manage_message, fbdev_flush, evdev_read);
#endif
#if USE_SDL
  sdl_init();
  view_init(&model, controller_manage_message, sdl_display_flush,
            sdl_mouse_read);
#endif

  model_init(&model);
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
