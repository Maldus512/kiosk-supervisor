#include "lvgl.h"
#include "style.h"


static const lv_style_const_prop_t style_transparent_cont_props[] = {
    LV_STYLE_CONST_BG_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_RADIUS(0),
    LV_STYLE_CONST_BORDER_WIDTH(0),
};
LV_STYLE_CONST_INIT(style_transparent_cont, style_transparent_cont_props);


static const lv_style_const_prop_t style_panel_props[] = {
    LV_STYLE_CONST_PAD_TOP(16),
    LV_STYLE_CONST_PAD_BOTTOM(16),
    LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PAD_RIGHT(8),
};
LV_STYLE_CONST_INIT(style_panel, style_panel_props);


lv_style_t style_icon = {0};


void style_init(void) {
    lv_style_init(&style_icon);
    lv_style_set_img_recolor_opa(&style_icon, LV_OPA_COVER);
    lv_style_set_img_recolor(&style_icon, STYLE_FG_COLOR);
}