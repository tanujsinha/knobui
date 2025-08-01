// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: Smartwatch

#ifndef UI_LEDCOLOR_H
#define UI_LEDCOLOR_H

#ifdef __cplusplus
extern "C" {
#endif

// SCREEN: ui_LedColor
extern void ui_LedColor_screen_init(void);
extern void ui_LedColor_screen_destroy(void);
extern void ui_LedColor_update_color(int16_t hue_delta);
extern lv_obj_t * ui_LedColor;
extern lv_obj_t * ui_LedColorWheel;
extern void ui_event_BackButtonLed(lv_event_t * e);
extern lv_obj_t * ui_BackButtonLed;
extern lv_obj_t * ui_ButtonName;
// CUSTOM VARIABLES

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

