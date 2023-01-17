#include "lv_page_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "log.h"


enum {
    BTN_ID,
};


struct page_data {};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(void *args, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    return pdata;
}


static void open_page(lv_pman_handle_t handle, void *args, void *state) {
    struct page_data *pdata  = state;
    model_t          *pmodel = args;

    (void)pdata;
    (void)pmodel;

    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 96, 64);
    lv_obj_center(btn);
    lv_pman_register_obj_id(handle, btn, BTN_ID);

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
                        case BTN_ID:
                            log_info("Clicked!");
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        case LV_PMAN_EVENT_TAG_USER: {
            switch (event.user_event.tag) {
                case LV_PMAN_USER_EVENT_TAG_UPDATE:
                    update_page(pmodel, pdata);
                    break;
            }
            break;
        }
    }

    return msg;
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    (void)pmodel;
    (void)pdata;
}


const lv_pman_page_t page_main = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};