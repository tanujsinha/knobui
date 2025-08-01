// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: Smartwatch

#ifndef _UI_COMP__H
#define _UI_COMP__H

#include "../ui.h"

#ifdef __cplusplus
extern "C" {
#endif

void get_component_child_event_cb(lv_event_t* e);
void del_component_child_event_cb(lv_event_t* e);

lv_obj_t * ui_comp_get_child(lv_obj_t *comp, uint32_t child_idx);
extern uint32_t LV_EVENT_GET_COMP_CHILD;
#include "ui_comp_batterygroup.h"
#include "ui_comp_buttondown.h"
#include "ui_comp_buttonround.h"
#include "ui_comp_buttontop.h"
#include "ui_comp_dailymissiongroup.h"
#include "ui_comp_dategroup.h"
#include "ui_comp_forecastgroup.h"
#include "ui_comp_pulsegroup.h"
#include "ui_comp_stepgroup.h"
#include "ui_comp_titlegroup.h"
#include "ui_comp_todayweathergroup.h"
#include "ui_comp_weathergroup1.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
