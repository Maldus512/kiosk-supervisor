#include <stdio.h>
#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "lv_page_manager.h"


void controller_init(model_t *pmodel) {
    view_change_page(pmodel, page_main);
}


void controller_manage_message(void *args, lv_pman_controller_msg_t msg) {
    model_t *pmodel = args;

    switch (msg.tag) {
        case LV_PMAN_CONTROLLER_MSG_TAG_NONE:
            break;
    }
}


void controller_manage(model_t *pmodel) {}