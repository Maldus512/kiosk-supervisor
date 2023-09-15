#include "controller/message.h"
#include "log.h"
#include "lv_page_manager.h"
#include "lv_page_manager_conf.h"
#include "model/model.h"
#include "src/core/lv_disp.h"
#include "src/core/lv_obj_pos.h"
#include "src/extra/widgets/msgbox/lv_msgbox.h"
#include "src/page.h"
#include "view/view.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum {
    BTN_SETTINGS = 100,
};
struct page_data {};

static void update_page(model_t *pmodel, struct page_data *pdata);

static void open_page(lv_pman_handle_t handle, void *args, void *state) {
    struct page_data *pdata  = state;
    model_t          *pmodel = args;

    (void)pdata;
    (void)pmodel;

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa(&style, LV_OPA_0);

    lv_obj_t *label;
    lv_obj_t *btn_settings = lv_btn_create(lv_scr_act());
    lv_obj_align(btn_settings, LV_ALIGN_TOP_RIGHT, 0, 0);

    label = lv_label_create(btn_settings);
    lv_label_set_text(label, "");
    lv_obj_center(label);
    lv_obj_add_style(btn_settings, &style, LV_PART_MAIN);
    lv_pman_register_obj_id(handle, btn_settings, BTN_SETTINGS);

    update_page(pmodel, pdata);
}

static lv_pman_msg_t process_page_event(void *args, void *state, lv_pman_event_t event) {
    lv_pman_msg_t     msg    = {0};
    model_t          *pmodel = args;
    struct page_data *pdata  = state;

    switch (event.tag) {
        case LV_PMAN_EVENT_TAG_OPEN:
            break;

        case LV_PMAN_EVENT_TAG_LVGL: {
            switch (event.lvgl.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.lvgl.id) {
                        case BTN_SETTINGS:
                            view_open_settings_event();
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
                case LV_PMAN_USER_EVENT_TAG_OPEN_SETTINGS:
                    msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_OPEN_SETTINGS;
                    break;
            }
            break;
        }
    }

    return msg;
}

static void *create_page(void *args, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    return pdata;
}

static void update_page(model_t *pmodel, struct page_data *pdata) {
    (void)pmodel;
    (void)pdata;
}

const lv_pman_page_t page_black = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};
