#include "controller/controller.h"
#include "controller/message.h"
#include "log.h"
#include "lv_page_manager.h"
#include "lv_page_manager_conf.h"
#include "model/model.h"
#include "src/core/lv_disp.h"
#include "src/core/lv_obj_pos.h"
#include "src/extra/widgets/msgbox/lv_msgbox.h"
#include "src/misc/lv_color.h"
#include "src/page.h"
#include "view/view.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum {
  BTN_NEW_VERSION = 0,
  BTN_EXPORT,
  BTN_START_APP,
  BTN_START_APP_RESTART,
  BTN_START_APP_CANCEL,
  BTN_SUP_VERSION,
  BTN_APP_VERSION,
  BTN_CLOSE,
  BTN_CLOSE_MSGBOX,
  BTN_UPDATE_FOUND_OK,
  BTN_UPDATE_FOUND_CANCEL,
};

struct page_data {
  bool msgbox_showing;
  lv_obj_t *msgbox;
  lv_obj_t *msgbox_sup_version;
  lv_obj_t *msgbox_app_version;
  lv_obj_t *msgbox_app_start_already;
  lv_obj_t *msgbox_app_start_succ;
  lv_obj_t *msgbox_app_start_err;
  lv_obj_t *msgbox_app_exit_err;
  lv_obj_t *msgbox_app_exit_many_times;
  lv_obj_t *msgbox_export_logs_succ;
  lv_obj_t *msgbox_export_logs_err;

  lv_obj_t *msgbox_update_found;
  lv_obj_t *msgbox_update_already;
  lv_obj_t *msgbox_update_succ;
  lv_obj_t *msgbox_update_err;
};

static void update_page(model_t *pmodel, struct page_data *pdata);
static void show_msgbox(model_t *pmodel, struct page_data *pdata,
                        lv_obj_t *msgbox);
static void hide_msgbox(model_t *pmodel, struct page_data *pdata);

static void *create_page(void *args, void *extra) {
  struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
  assert(pdata != NULL);

  pdata->msgbox = NULL;
  pdata->msgbox_showing = false;

  return pdata;
}

static void create_menu_btn(lv_pman_handle_t handle, int btn_id, char *text,
                            int y) {
  lv_obj_t *label;
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, y);

  label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_center(label);
  lv_pman_register_obj_id(handle, btn, btn_id);
}

static lv_obj_t *my_msgbox_create(lv_pman_handle_t handle,
                                  struct msgbox_t msgbox) {
  const uint32_t MSGW = 600, MSGH = 300;

  LV_LOG_INFO("begin");
  if (msgbox.obj == NULL) {
    *msgbox.obj =
        lv_obj_class_create_obj(&lv_msgbox_backdrop_class, lv_layer_top());
    LV_ASSERT_MALLOC(msgbox.obj);
    lv_obj_class_init_obj(*msgbox.obj);
    lv_obj_clear_flag(*msgbox.obj, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(*msgbox.obj, LV_PCT(100), LV_PCT(100));
  }

  lv_obj_t *obj = lv_obj_class_create_obj(&lv_msgbox_class, *msgbox.obj);
  LV_ASSERT_MALLOC(obj);
  if (obj == NULL)
    return NULL;
  lv_obj_class_init_obj(obj);
  lv_obj_set_width(obj, MSGW);
  lv_obj_set_height(obj, MSGH);

  lv_msgbox_t *mbox = (lv_msgbox_t *)obj;

  lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);

  bool has_title = msgbox.title && strlen(msgbox.title) > 0;

  /*When a close button is required, we need the empty label as spacer to push
   * the button to the right*/
  if (msgbox.btn_close || has_title) {
    mbox->title = lv_label_create(obj);
    lv_label_set_text(mbox->title, has_title ? msgbox.title : "");
    lv_label_set_long_mode(mbox->title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    if (msgbox.btn_close)
      lv_obj_set_flex_grow(mbox->title, 1);
    else
      lv_obj_set_width(mbox->title, LV_PCT(100));
  }

  if (msgbox.btn_close) {
    mbox->close_btn = lv_btn_create(obj);
    lv_obj_set_ext_click_area(mbox->close_btn, LV_DPX(10));
    lv_pman_register_obj_id(handle, mbox->close_btn, BTN_CLOSE_MSGBOX);
    lv_obj_t *label = lv_label_create(mbox->close_btn);
    lv_label_set_text(label, LV_SYMBOL_CLOSE);
    const lv_font_t *font =
        lv_obj_get_style_text_font(mbox->close_btn, LV_PART_MAIN);
    lv_coord_t close_btn_size = lv_font_get_line_height(font) + LV_DPX(10);
    lv_obj_set_size(mbox->close_btn, close_btn_size, close_btn_size);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  }

  mbox->content = lv_obj_class_create_obj(&lv_msgbox_content_class, obj);

  bool has_txt = msgbox.text && strlen(msgbox.text) > 0;
  if (has_txt) {
    mbox->text = lv_label_create(mbox->content);
    lv_label_set_text(mbox->text, msgbox.text);
    lv_label_set_long_mode(mbox->text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(mbox->text, lv_pct(100));
  }

  if (msgbox.btn_texts) {
    uint32_t btn_cnt = 0;
    while (msgbox.btn_texts[btn_cnt] && msgbox.btn_texts[btn_cnt][0] != '\0') {
      btn_cnt++;
    }

    const uint32_t btn_w = (MSGW - LV_DPX(75)) / btn_cnt;
    for (uint8_t i = 0; i < btn_cnt; i++) {
      lv_obj_t *btn = lv_btn_create(obj);
      lv_pman_register_obj_id(handle, btn, msgbox.btn_ids[i]);
      lv_obj_t *label = lv_label_create(btn);
      lv_label_set_text(label, msgbox.btn_texts[i]);
      const lv_font_t *font = lv_obj_get_style_text_font(btn, LV_PART_MAIN);
      lv_coord_t btn_h = lv_font_get_line_height(font) + LV_DPX(10);
      lv_obj_set_size(btn, btn_w, btn_h);
      lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    }
  }

  return obj;
}

// static void create_msgbox(lv_pman_handle_t handle, lv_obj_t **obj,
//                           const char *title, const char *text,
//                           const int btn_ids[], const char *btn_txts[],
//                           const bool add_close_btn) {
static void create_msgbox(lv_pman_handle_t handle, struct msgbox_t msgbox) {
  *msgbox.obj = lv_obj_create(lv_scr_act());
  lv_obj_set_pos(*msgbox.obj, 0, 0);
  lv_obj_set_size(*msgbox.obj, LV_HOR_RES, LV_VER_RES);
  lv_color_t c = lv_palette_main(LV_PALETTE_DEEP_PURPLE);
  lv_obj_set_style_bg_color(*msgbox.obj, c, LV_STATE_DEFAULT);

  // lv_obj_t *btn = my_msgbox_create(handle, *obj, title, text, btn_ids,
  // btn_txts,
  //                                  add_close_btn);
  lv_obj_t *btn = my_msgbox_create(handle, msgbox);

  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/

  lv_obj_add_flag(*msgbox.obj, LV_OBJ_FLAG_HIDDEN);
}

static void change_label_msgbox(lv_obj_t *obj, const char *text) {
  lv_msgbox_t *mbox = (lv_msgbox_t *)lv_obj_get_child(obj, 0);
  lv_label_set_text(mbox->text, text);
}

static void open_page(lv_pman_handle_t handle, void *args, void *state) {
  struct page_data *pdata = state;
  model_t *pmodel = args;

  (void)pdata;
  (void)pmodel;

  int y = -160;

  int btn_n = 6;
  int btn_ids[] = {BTN_NEW_VERSION, BTN_EXPORT,      BTN_START_APP,
                   BTN_SUP_VERSION, BTN_APP_VERSION, BTN_CLOSE};
  char *btn_texts[] = {"New version",           "Export diagnose files",
                       "Start the application", "Supervisor version",
                       "Application version",   "Close"};

  int msgboxes_n = 13;

  struct msgbox_t m_sup_ver = {.obj = &pdata->msgbox_sup_version,
                               .title = "Supervisor version",
                               .text = model_get_sup_version(pmodel),
                               .btn_close = true,
                               .btn_ids = NULL,
                               .btn_texts = NULL};
  struct msgbox_t m_app_ver = {.obj = &pdata->msgbox_app_version,
                               .title = "Application version",
                               .text = model_get_app_version(pmodel),
                               .btn_close = true,
                               .btn_ids = NULL,
                               .btn_texts = NULL};

  static int msgbox_start_already_btn_ids[] = {BTN_START_APP_RESTART,
                                               BTN_START_APP_CANCEL};
  static char *msgbox_start_already_btn_texts[] = {"RESTART", "CANCEL", "\0"};
  struct msgbox_t m_app_start_already = {
      .obj = &pdata->msgbox_app_start_already,
      .title = "Application",
      .text = "Application is already running, do you want to restart it ?",
      .btn_close = false,
      .btn_ids = msgbox_start_already_btn_ids,
      .btn_texts = msgbox_start_already_btn_texts};

  struct msgbox_t m_app_start_succ = {.obj = &pdata->msgbox_app_start_succ,
                                      .title = "Application status",
                                      .text =
                                          "Application successfully started",
                                      .btn_close = true,
                                      .btn_ids = NULL,
                                      .btn_texts = NULL};
  struct msgbox_t m_app_start_err = {
      .obj = &pdata->msgbox_app_start_err,
      .title = "Application status",
      .text =
          "Failed to start application\nPlease check that the path is correct",
      .btn_close = true,
      .btn_ids = NULL,
      .btn_texts = NULL};
  struct msgbox_t m_app_exit_err = {
      .obj = &pdata->msgbox_app_exit_err,
      .title = "Error",
      .text = "The application terminated with an error !!!",
      .btn_close = true,
      .btn_ids = NULL,
      .btn_texts = NULL};
  char many_txt[STRSIZE];
  sprintf(many_txt,
          "The application successfully terminated at least %d times in less "
          "than %d ms",
          model_get_term_per_period(pmodel), model_get_period(pmodel));
  struct msgbox_t m_app_exit_many_times = {
      .obj = &pdata->msgbox_app_exit_many_times,
      .title = "Error",
      .text = many_txt,
      .btn_close = true,
      .btn_ids = NULL,
      .btn_texts = NULL};
  char export_logs_txt[100 + strlen(model_get_mount_path(pmodel))];
  sprintf(export_logs_txt, "Logs successfully exported in\n%s",
          model_get_mount_path(pmodel));
  struct msgbox_t m_export_logs_succ = {.obj = &pdata->msgbox_export_logs_succ,
                                        .title = "Export",
                                        .text = "Logs successfully exported",
                                        .btn_close = true,
                                        .btn_ids = NULL,
                                        .btn_texts = NULL};
  struct msgbox_t m_export_logs_err = {.obj = &pdata->msgbox_export_logs_err,
                                       .title = "Export",
                                       .text = "Failed to export logs",
                                       .btn_close = true,
                                       .btn_ids = NULL,
                                       .btn_texts = NULL};
  static int msgbox_update_btn_ids[] = {BTN_UPDATE_FOUND_OK,
                                        BTN_UPDATE_FOUND_CANCEL};
  static char *msgbox_update_btn_texts[] = {"UPDATE", "CANCEL", "\0"};
  struct msgbox_t m_update_found = {
      .obj = &pdata->msgbox_update_found,
      .title = "Update found",
      .text = "Do you want to update the application?",
      .btn_close = false,
      .btn_ids = msgbox_update_btn_ids,
      .btn_texts = msgbox_update_btn_texts};
  struct msgbox_t m_update_already = {.obj = &pdata->msgbox_update_already,
                                      .title = "Update",
                                      .text = "Application already updated",
                                      .btn_close = true,
                                      .btn_ids = NULL,
                                      .btn_texts = NULL};
  struct msgbox_t m_update_succ = {.obj = &pdata->msgbox_update_succ,
                                   .title = "Update",
                                   .text = "Update successfully installed",
                                   .btn_close = true,
                                   .btn_ids = NULL,
                                   .btn_texts = NULL};
  char update_path_txt[100 + strlen(model_get_update_path(pmodel))];
  sprintf(update_path_txt, "Update failed, check that the file exists in \n%s",
          model_get_update_path(pmodel));
  struct msgbox_t m_update_err = {.obj = &pdata->msgbox_update_err,
                                  .title = "Update",
                                  .text = update_path_txt,
                                  .btn_close = true,
                                  .btn_ids = NULL,
                                  .btn_texts = NULL};

  struct msgbox_t msgboxes[] = {m_sup_ver,
                                m_app_ver,
                                m_app_start_already,
                                m_app_start_succ,
                                m_app_start_err,
                                m_app_exit_err,
                                m_app_exit_many_times,
                                m_export_logs_succ,
                                m_export_logs_err,
                                m_update_found,
                                m_update_already,
                                m_update_succ,
                                m_update_err};

  for (uint8_t i = 0; i < btn_n; i++) {
    create_menu_btn(handle, btn_ids[i], btn_texts[i], y);
    y += 60;
  }

  for (uint8_t i = 0; i < msgboxes_n; i++) {
    create_msgbox(handle, msgboxes[i]);
  }

  update_page(pmodel, pdata);
}

static lv_pman_msg_t process_page_event(void *args, void *state,
                                        lv_pman_event_t event) {
  lv_pman_msg_t msg = {0};
  model_t *pmodel = args;
  struct page_data *pdata = state;

  switch (event.tag) {
  case LV_PMAN_EVENT_TAG_OPEN:
    model_set_settings_open(pmodel, true);
    break;

  case LV_PMAN_EVENT_TAG_LVGL: {
    switch (event.lvgl.event) {
    case LV_EVENT_CLICKED: {
      switch (event.lvgl.id) {
      case BTN_NEW_VERSION:
        log_info("New version!");
        view_app_update_event();
        break;
      case BTN_EXPORT:
        view_export_logs_event();
        break;
      case BTN_START_APP:
        view_app_start_event();
        break;
      case BTN_START_APP_RESTART:
        log_info("Restarting");
        controller_stop_app(pmodel);
        controller_start_app(pmodel);
        break;
      case BTN_START_APP_CANCEL:
        hide_msgbox(pmodel, pdata);
        break;
      case BTN_SUP_VERSION:
        show_msgbox(pmodel, pdata, pdata->msgbox_sup_version);
        break;
      case BTN_APP_VERSION:
        show_msgbox(pmodel, pdata, pdata->msgbox_app_version);
        log_info("ver (%s)", model_get_app_version(pmodel));
        change_label_msgbox(pdata->msgbox, model_get_app_version(pmodel));
        break;
      case BTN_CLOSE:
        _Exit(0);
        break;
      case BTN_CLOSE_MSGBOX:
        pdata->msgbox_showing = false;
        break;
      case BTN_UPDATE_FOUND_OK:
        controller_update_app(pmodel);
        model_set_msgbox_update_open(pmodel, false);
        break;
      case BTN_UPDATE_FOUND_CANCEL:
        hide_msgbox(pmodel, pdata);
        model_set_msgbox_update_open(pmodel, false);
        break;
      }
      update_page(pmodel, pdata);
      break;
    }

    default:
      break;
    }
    break;
  }

  case LV_PMAN_EVENT_TAG_USER: {
    switch (event.user_event.tag) {
    case LV_PMAN_USER_EVENT_TAG_APP_START:
      msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_APP_START;
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_START_ALREADY:
      show_msgbox(pmodel, pdata, pdata->msgbox_app_start_already);
      break;
    case LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS:
      msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS;
      break;
    case LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS_SUCC:
      show_msgbox(pmodel, pdata, pdata->msgbox_export_logs_succ);
      break;
    case LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS_ERR:
      show_msgbox(pmodel, pdata, pdata->msgbox_export_logs_err);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_START_SUCC:
      show_msgbox(pmodel, pdata, pdata->msgbox_app_start_succ);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_START_ERR:
      show_msgbox(pmodel, pdata, pdata->msgbox_app_start_err);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_EXIT_ERR:
      show_msgbox(pmodel, pdata, pdata->msgbox_app_exit_err);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_EXIT_MANY_TIMES:
      show_msgbox(pmodel, pdata, pdata->msgbox_app_exit_many_times);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_VERSION:
      msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_APP_VERSION;
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_UPDATE:
      msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_APP_UPDATE;
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_UPDATE_ALREADY:
      show_msgbox(pmodel, pdata, pdata->msgbox_update_already);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_UPDATE_FOUND:
      show_msgbox(pmodel, pdata, pdata->msgbox_update_found);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_UPDATE_SUCC:
      show_msgbox(pmodel, pdata, pdata->msgbox_update_succ);
      break;
    case LV_PMAN_USER_EVENT_TAG_APP_UPDATE_ERR:
      show_msgbox(pmodel, pdata, pdata->msgbox_update_err);
      break;
    }
    break;
  }
  }

  return msg;
}

static void show_msgbox(model_t *pmodel, struct page_data *pdata,
                        lv_obj_t *msgbox) {
  // hide previous msgbox
  hide_msgbox(pmodel, pdata);

  pdata->msgbox = msgbox;
  pdata->msgbox_showing = true;
  update_page(pmodel, pdata);
}
static void hide_msgbox(model_t *pmodel, struct page_data *pdata) {
  pdata->msgbox = NULL;
  pdata->msgbox_showing = false;
  update_page(pmodel, pdata);
}

static void update_page(model_t *pmodel, struct page_data *pdata) {
  (void)pmodel;
  (void)pdata;

  if (pdata->msgbox != NULL) {
    if (pdata->msgbox_showing)
      lv_obj_clear_flag(pdata->msgbox, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(pdata->msgbox, LV_OBJ_FLAG_HIDDEN);
  }
}

const lv_pman_page_t page_settings = {
    .create = create_page,
    .destroy = lv_pman_destroy_all,
    .open = open_page,
    .close = lv_pman_close_all,
    .process_event = process_page_event,
};
