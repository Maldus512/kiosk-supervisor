#ifndef CONTROLLER_MESSAGE_H_INCLUDED
#define CONTROLLER_MESSAGE_H_INCLUDED

#include <stdint.h>


typedef enum {
    LV_PMAN_CONTROLLER_MSG_TAG_NONE = 0,
} lv_pman_controller_msg_tag_t;


typedef struct {
    lv_pman_controller_msg_tag_t tag;

    union {
        uint16_t test;
    };
} lv_pman_controller_msg_t;


#endif