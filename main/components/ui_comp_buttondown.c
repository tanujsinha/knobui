// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: Smartwatch

#include "../ui.h"

// COMPONENT buttondown

lv_obj_t *ui_buttondown_create(lv_obj_t *comp_parent) {

lv_obj_t *cui_buttondown;
cui_buttondown = lv_obj_create(comp_parent);
lv_obj_set_width( cui_buttondown, 70);
lv_obj_set_height( cui_buttondown, 70);
lv_obj_set_x( cui_buttondown, 146 );
lv_obj_set_y( cui_buttondown, 102 );
lv_obj_set_align( cui_buttondown, LV_ALIGN_CENTER );
lv_obj_clear_flag( cui_buttondown, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_radius(cui_buttondown, 200, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(cui_buttondown, lv_color_hex(0xEE1E1E), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_buttondown, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(cui_buttondown, lv_color_hex(0xEE1E1E), LV_PART_MAIN | LV_STATE_CHECKED );
lv_obj_set_style_bg_opa(cui_buttondown, 255, LV_PART_MAIN| LV_STATE_CHECKED);
lv_obj_set_style_bg_color(cui_buttondown, lv_color_hex(0xEE1E1E), LV_PART_MAIN | LV_STATE_PRESSED );
lv_obj_set_style_bg_opa(cui_buttondown, 255, LV_PART_MAIN| LV_STATE_PRESSED);

lv_obj_t *cui_button_down_icon;
cui_button_down_icon = lv_img_create(cui_buttondown);
lv_img_set_src(cui_button_down_icon, &ui_img_weather_png);
lv_obj_set_width( cui_button_down_icon, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_button_down_icon, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( cui_button_down_icon, -12 );
lv_obj_set_y( cui_button_down_icon, -9 );
lv_obj_set_align( cui_button_down_icon, LV_ALIGN_CENTER );
lv_obj_add_flag( cui_button_down_icon, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( cui_button_down_icon, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

lv_obj_t *cui_line;
cui_line = lv_img_create(cui_buttondown);
lv_img_set_src(cui_line, &ui_img_btn_bg_3_png);
lv_obj_set_width( cui_line, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_line, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( cui_line, 8 );
lv_obj_set_y( cui_line, 7 );
lv_obj_set_align( cui_line, LV_ALIGN_CENTER );
lv_obj_add_flag( cui_line, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( cui_line, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

lv_obj_t ** children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_BUTTONDOWN_NUM);
children[UI_COMP_BUTTONDOWN_BUTTONDOWN] = cui_buttondown;
children[UI_COMP_BUTTONDOWN_BUTTON_DOWN_ICON] = cui_button_down_icon;
children[UI_COMP_BUTTONDOWN_LINE] = cui_line;
lv_obj_add_event_cb(cui_buttondown, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
lv_obj_add_event_cb(cui_buttondown, del_component_child_event_cb, LV_EVENT_DELETE, children);
ui_comp_buttondown_create_hook(cui_buttondown);
return cui_buttondown; 
}

