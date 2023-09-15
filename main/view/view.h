#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include "lv_page_manager.h"
#include "model/model.h"

void view_init(model_t *pmodel, void (*controller_cb)(void *, lv_pman_controller_msg_t));
void view_change_page(model_t *pmodel, lv_pman_page_t page);
void view_user_event(lv_pman_user_event_tag_t tag);

void view_app_start_event();
void view_app_start_already_event();

void view_app_start_success_event();
void view_app_start_error_event();
void view_app_exit_error_event();
void view_app_exit_many_times_event();
void view_app_version_event();

void view_app_update_event();
void view_app_update_already_event();
void view_app_update_found_event();
void view_app_update_success_event();
void view_app_update_error_event();

void view_export_logs_event();
void view_export_logs_success_event();
void view_export_logs_error_event();

void view_open_settings_event();

extern const lv_pman_page_t page_black;
extern const lv_pman_page_t page_settings;

#endif
