// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: Smartwatch

#include "../ui.h"

// COMPONENT buttontop

lv_obj_t *ui_buttontop_create(lv_obj_t *comp_parent) {

lv_obj_t *cui_buttontop;
cui_buttontop = lv_obj_create(comp_parent);
lv_obj_set_width( cui_buttontop, 70);
lv_obj_set_height( cui_buttontop, 70);
lv_obj_set_x( cui_buttontop, 145 );
lv_obj_set_y( cui_buttontop, -105 );
lv_obj_set_align( cui_buttontop, LV_ALIGN_CENTER );
lv_obj_clear_flag( cui_buttontop, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_radius(cui_buttontop, 200, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(cui_buttontop, lv_color_hex(0xEE1E1E), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_buttontop, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(cui_buttontop, lv_color_hex(0xEE1E1E), LV_PART_MAIN | LV_STATE_CHECKED );
lv_obj_set_style_bg_opa(cui_buttontop, 255, LV_PART_MAIN| LV_STATE_CHECKED);
lv_obj_set_style_bg_color(cui_buttontop, lv_color_hex(0xEE1E1E), LV_PART_MAIN | LV_STATE_PRESSED );
lv_obj_set_style_bg_opa(cui_buttontop, 255, LV_PART_MAIN| LV_STATE_PRESSED);

lv_obj_t *cui_button_top_icon;
cui_button_top_icon = lv_img_create(cui_buttontop);
lv_img_set_src(cui_button_top_icon, &ui_img_call1_png);
lv_obj_set_width( cui_button_top_icon, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_button_top_icon, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( cui_button_top_icon, -10 );
lv_obj_set_y( cui_button_top_icon, 7 );
lv_obj_set_align( cui_button_top_icon, LV_ALIGN_CENTER );
lv_obj_add_flag( cui_button_top_icon, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( cui_button_top_icon, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

lv_obj_t *cui_line;
cui_line = lv_img_create(cui_buttontop);
lv_img_set_src(cui_line, &ui_img_btn_bg_1_png);
lv_obj_set_width( cui_line, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_line, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( cui_line, 9 );
lv_obj_set_y( cui_line, -3 );
lv_obj_set_align( cui_line, LV_ALIGN_CENTER );
lv_obj_add_flag( cui_line, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( cui_line, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

lv_obj_t ** children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_BUTTONTOP_NUM);
children[UI_COMP_BUTTONTOP_BUTTONTOP] = cui_buttontop;
children[UI_COMP_BUTTONTOP_BUTTON_TOP_ICON] = cui_button_top_icon;
children[UI_COMP_BUTTONTOP_LINE] = cui_line;
lv_obj_add_event_cb(cui_buttontop, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
lv_obj_add_event_cb(cui_buttontop, del_component_child_event_cb, LV_EVENT_DELETE, children);
ui_comp_buttontop_create_hook(cui_buttontop);
return cui_buttontop; 
}

