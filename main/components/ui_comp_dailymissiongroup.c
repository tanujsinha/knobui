// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: Smartwatch

#include "../ui.h"

void ui_event_comp_dailymissiongroup_daily_mission_arc( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
lv_obj_t **comp_dailymissiongroup = lv_event_get_user_data(e);

if ( event_code == LV_EVENT_VALUE_CHANGED) {
      _ui_arc_set_text_value( comp_dailymissiongroup[UI_COMP_DAILYMISSIONGROUP_MISSION_PERCENT], target, "", "%");
}
}

// COMPONENT dailymissiongroup

lv_obj_t *ui_dailymissiongroup_create(lv_obj_t *comp_parent) {

lv_obj_t *cui_dailymissiongroup;
cui_dailymissiongroup = lv_obj_create(comp_parent);
lv_obj_set_width( cui_dailymissiongroup, 283);
lv_obj_set_height( cui_dailymissiongroup, 185);
lv_obj_set_align( cui_dailymissiongroup, LV_ALIGN_BOTTOM_LEFT );
lv_obj_clear_flag( cui_dailymissiongroup, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_bg_color(cui_dailymissiongroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_dailymissiongroup, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t *cui_mission_percent;
cui_mission_percent = lv_label_create(cui_dailymissiongroup);
lv_obj_set_width( cui_mission_percent, LV_SIZE_CONTENT);  /// 90
lv_obj_set_height( cui_mission_percent, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( cui_mission_percent, 207 );
lv_obj_set_y( cui_mission_percent, -17 );
lv_obj_set_align( cui_mission_percent, LV_ALIGN_BOTTOM_LEFT );
lv_label_set_text(cui_mission_percent,"87%");
lv_obj_set_style_text_color(cui_mission_percent, lv_color_hex(0xEE1E1E), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(cui_mission_percent, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(cui_mission_percent, &ui_font_Subtitle, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t *cui_daily_mission_image;
cui_daily_mission_image = lv_img_create(cui_dailymissiongroup);
lv_img_set_src(cui_daily_mission_image, &ui_img_daily_mission_png);
lv_obj_set_width( cui_daily_mission_image, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_daily_mission_image, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( cui_daily_mission_image, -58 );
lv_obj_set_y( cui_daily_mission_image, -23 );
lv_obj_set_align( cui_daily_mission_image, LV_ALIGN_CENTER );
lv_obj_add_flag( cui_daily_mission_image, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( cui_daily_mission_image, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

lv_obj_t *cui_daily_mission_arc;
cui_daily_mission_arc = lv_arc_create(cui_dailymissiongroup);
lv_obj_set_width( cui_daily_mission_arc, 330);
lv_obj_set_height( cui_daily_mission_arc, 330);
lv_obj_set_x( cui_daily_mission_arc, 50 );
lv_obj_set_y( cui_daily_mission_arc, -98 );
lv_obj_set_align( cui_daily_mission_arc, LV_ALIGN_CENTER );
lv_obj_add_flag( cui_daily_mission_arc, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_arc_set_value(cui_daily_mission_arc, 87);
lv_arc_set_bg_angles(cui_daily_mission_arc,88,170);
lv_obj_set_style_arc_color(cui_daily_mission_arc, lv_color_hex(0x303030), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_arc_opa(cui_daily_mission_arc, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_arc_rounded(cui_daily_mission_arc, true, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_set_style_arc_color(cui_daily_mission_arc, lv_color_hex(0xB0E14A), LV_PART_INDICATOR | LV_STATE_DEFAULT );
lv_obj_set_style_arc_opa(cui_daily_mission_arc, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);
lv_obj_set_style_arc_rounded(cui_daily_mission_arc, true, LV_PART_INDICATOR| LV_STATE_DEFAULT);

lv_obj_set_style_bg_color(cui_daily_mission_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_daily_mission_arc, 0, LV_PART_KNOB| LV_STATE_DEFAULT);

lv_obj_t ** children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_DAILYMISSIONGROUP_NUM);
children[UI_COMP_DAILYMISSIONGROUP_DAILYMISSIONGROUP] = cui_dailymissiongroup;
children[UI_COMP_DAILYMISSIONGROUP_MISSION_PERCENT] = cui_mission_percent;
children[UI_COMP_DAILYMISSIONGROUP_DAILY_MISSION_IMAGE] = cui_daily_mission_image;
children[UI_COMP_DAILYMISSIONGROUP_DAILY_MISSION_ARC] = cui_daily_mission_arc;
lv_obj_add_event_cb(cui_dailymissiongroup, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
lv_obj_add_event_cb(cui_dailymissiongroup, del_component_child_event_cb, LV_EVENT_DELETE, children);
lv_obj_add_event_cb(cui_daily_mission_arc, ui_event_comp_dailymissiongroup_daily_mission_arc, LV_EVENT_ALL, children);
ui_comp_dailymissiongroup_create_hook(cui_dailymissiongroup);
return cui_dailymissiongroup; 
}

