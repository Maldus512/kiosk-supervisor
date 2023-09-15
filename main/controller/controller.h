#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include "lv_page_manager.h"
#include "model/model.h"

struct msgbox_t {
    lv_obj_t **obj;
    char      *title;
    char      *text;
    bool       btn_close;
    int       *btn_ids;
    char     **btn_texts;
};

bool copy_file(const char *source_path, const char *dest_path);
bool file_exe(const char *path);
void get_app_version(model_t *pmodel, const char *path, char *buffer);

void controller_init(model_t *pmodel);
void controller_manage_message(void *args, lv_pman_controller_msg_t msg);
void controller_manage(model_t *pmodel);
void controller_start_app(model_t *pmodel);
void controller_stop_app(model_t *pmodel);
void controller_export_logs(model_t *pmodel);
void controller_get_app_version(model_t *pmodel);
void controller_update_app(model_t *pmodel);
void controller_open_settings(model_t *pmodel);

#endif
