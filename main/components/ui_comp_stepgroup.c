// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: Smartwatch

#include "../ui.h"

// COMPONENT stepgroup

lv_obj_t *ui_stepgroup_create(lv_obj_t *comp_parent) {

lv_obj_t *cui_stepgroup;
cui_stepgroup = lv_obj_create(comp_parent);
lv_obj_set_width( cui_stepgroup, 100);
lv_obj_set_height( cui_stepgroup, 26);
lv_obj_set_x( cui_stepgroup, 0 );
lv_obj_set_y( cui_stepgroup, -40 );
lv_obj_set_align( cui_stepgroup, LV_ALIGN_BOTTOM_MID );
lv_obj_set_flex_flow(cui_stepgroup,LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(cui_stepgroup, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_clear_flag( cui_stepgroup, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_bg_color(cui_stepgroup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_stepgroup, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_row(cui_stepgroup, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_column(cui_stepgroup, 8, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t *cui_step;
cui_step = lv_img_create(cui_stepgroup);
lv_img_set_src(cui_step, &ui_img_step_png);
lv_obj_set_width( cui_step, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_step, LV_SIZE_CONTENT);   /// 1
lv_obj_set_align( cui_step, LV_ALIGN_CENTER );
lv_obj_add_flag( cui_step, LV_OBJ_FLAG_ADV_HITTEST );   /// Flags
lv_obj_clear_flag( cui_step, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

lv_obj_t *cui_step_label;
cui_step_label = lv_label_create(cui_stepgroup);
lv_obj_set_width( cui_step_label, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_step_label, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( cui_step_label, 2 );
lv_obj_set_y( cui_step_label, -4 );
lv_obj_set_align( cui_step_label, LV_ALIGN_CENTER );
lv_label_set_text(cui_step_label,"1526");
lv_obj_set_style_text_color(cui_step_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(cui_step_label, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(cui_step_label, &ui_font_Title, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t ** children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_STEPGROUP_NUM);
children[UI_COMP_STEPGROUP_STEPGROUP] = cui_stepgroup;
children[UI_COMP_STEPGROUP_STEP] = cui_step;
children[UI_COMP_STEPGROUP_STEP_LABEL] = cui_step_label;
lv_obj_add_event_cb(cui_stepgroup, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
lv_obj_add_event_cb(cui_stepgroup, del_component_child_event_cb, LV_EVENT_DELETE, children);
ui_comp_stepgroup_create_hook(cui_stepgroup);
return cui_stepgroup; 
}

