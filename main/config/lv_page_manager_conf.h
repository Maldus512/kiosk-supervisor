#ifndef LV_PMAN_CONF_H_INCLUDED
#define LV_PMAN_CONF_H_INCLUDED

#include "controller/message.h"

#define LV_PMAN_PAGE_STACK_DEPTH 16

typedef enum {
    LV_PMAN_USER_EVENT_TAG_APP_START,
    LV_PMAN_USER_EVENT_TAG_APP_START_ALREADY,
    LV_PMAN_USER_EVENT_TAG_APP_START_SUCC,
    LV_PMAN_USER_EVENT_TAG_APP_START_ERR,
    LV_PMAN_USER_EVENT_TAG_APP_EXIT_ERR,
    LV_PMAN_USER_EVENT_TAG_APP_EXIT_MANY_TIMES,
    LV_PMAN_USER_EVENT_TAG_APP_VERSION,
    LV_PMAN_USER_EVENT_TAG_APP_UPDATE,
    LV_PMAN_USER_EVENT_TAG_APP_UPDATE_ALREADY,
    LV_PMAN_USER_EVENT_TAG_APP_UPDATE_FOUND,
    LV_PMAN_USER_EVENT_TAG_APP_UPDATE_SUCC,
    LV_PMAN_USER_EVENT_TAG_APP_UPDATE_ERR,

    LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS,
    LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS_SUCC,
    LV_PMAN_USER_EVENT_TAG_EXPORT_LOGS_ERR,

    LV_PMAN_USER_EVENT_TAG_OPEN_SETTINGS,

} lv_pman_user_event_tag_t;

typedef struct {
    lv_pman_user_event_tag_t tag;
} lv_pman_user_event_t;

#endif
