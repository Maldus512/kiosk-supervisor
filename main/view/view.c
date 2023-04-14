#include "view/view.h"
#include "lv_page_manager.h"
#include "lv_page_manager_conf.h"
#include "lvgl.h"
#include "model/model.h"
#include "src/misc/lv_color.h"
#if USE_EVDEV
#include "indev/evdev.h"
#endif
#if USE_FBDEV
#include "display/fbdev.h"
#endif
#if USE_SDL
#include "sdl/sdl.h"
#endif

#define WIN_HOR_RES 800
#define WIN_VER_RES 480

static lv_pman_t page_manager = {0};

void view_init(model_t *pmodel,
               void (*controller_cb)(void *, lv_pman_controller_msg_t),
               void (*flush_cb)(struct _lv_disp_drv_t *disp_drv,
                                const lv_area_t *area, lv_color_t *color_p),
               void (*read_cb)(struct _lv_indev_drv_t *indev_drv,
                               lv_indev_data_t *data)) {

#define BUFFER_SIZE (WIN_HOR_RES * WIN_VER_RES)
  /*A static or global variable to store the buffers*/
  static lv_disp_draw_buf_t disp_buf;

  /*Static or global buffer(s). The second buffer is optional*/
  static lv_color_t *buf_1[BUFFER_SIZE] = {0};

  /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL
   * instead buf_2 */
  lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, BUFFER_SIZE);

  static lv_disp_drv_t
      disp_drv; /*A variable to hold the drivers. Must be static or global.*/
  lv_disp_drv_init(&disp_drv);   /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf; /*Set an initialized buffer*/

  disp_drv.flush_cb = flush_cb; /*Set a flush callback to draw to the display*/

  disp_drv.hor_res = WIN_HOR_RES; /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = WIN_VER_RES; /*Set the vertical resolution in pixels*/

  lv_disp_t *disp = lv_disp_drv_register(
      &disp_drv); /*Register the driver and save the created display objects*/
  lv_theme_default_init(disp, lv_palette_darken(LV_PALETTE_PURPLE, 4),
                        lv_palette_main(LV_PALETTE_RED), 1, lv_font_default());

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv); /*Basic initialization*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;

  indev_drv.read_cb = read_cb;

  static lv_indev_t *indev = NULL;
  indev = lv_indev_drv_register(&indev_drv);

  lv_pman_init(&page_manager, pmodel, indev, controller_cb);
}

void view_change_page(model_t *pmodel, lv_pman_page_t page) {
  lv_pman_change_page(&page_manager, pmodel, page);
}

void view_app_start_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_START);
}
void view_app_start_already_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_START_ALREADY);
}
void view_app_start_success_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_START_SUCC);
}
void view_app_start_error_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_START_ERR);
}
void view_app_exit_error_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_EXIT_ERR);
}
void view_app_exit_many_times_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_EXIT_MANY_TIMES);
}
void view_app_version_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_VERSION);
}
void view_app_update_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_UPDATE);
}
void view_app_update_already_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_UPDATE_ALREADY);
}
void view_app_update_found_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_UPDATE_FOUND);
}
void view_app_update_success_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_UPDATE_SUCC);
}
void view_app_update_error_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_APP_UPDATE_ERR);
}

void view_export_logs_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS);
}
void view_export_logs_success_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS_SUCC);
}
void view_export_logs_error_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS_ERR);
}

void view_open_settings_event() {
  view_user_event(LV_PMAN_USER_EVENT_TAG_OPEN_SETTINGS);
}

void view_user_event(lv_pman_user_event_tag_t tag) {
  lv_pman_event(&page_manager, (lv_pman_event_t){.tag = LV_PMAN_EVENT_TAG_USER,
                                                 .user_event = {.tag = tag}});
}
