#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lvgl.h"
#include "esp_lcd_sh8601.h"
#include "i2c_bsp.h"
#include "lcd_touch_bsp.h"
#include "user_config.h"
#include "lcd_bl_pwm_bsp.h"
#include "user_encoder_bsp.h"
#include "i2c_equipment.h"

static const char *TAG = "counter";
static SemaphoreHandle_t lvgl_mux = NULL;

// Forward declarations
static void update_timer_display(void);
static void show_timer_setting_popup(void);
static void hide_timer_setting_popup(void);
static void update_timer_setting_display(void);
static void haptic_feedback_short(void);
static void start_timer_alarm(void);
static void stop_timer_alarm(void);
static void stop_timer_alarm_haptics_only(void);
static void alarm_haptic_callback(void* arg);
static void alarm_stop_callback(void* arg);
static void update_match_display(void);
static void show_match_popup(void);
static void hide_match_popup(void);
static void match_touch_cb(lv_event_t * e);
static void counter_touch_cb(lv_event_t * e);
static void update_battery_display(void);
static lv_color_t get_battery_color(int percentage);
static void match_button_cb(lv_event_t * e);
static void reset_match_data(void);
static void show_main_menu(void);
static void hide_main_menu(void);
static void update_main_menu_selection(void);
static void main_menu_select_cb(lv_event_t * e);
static void swipe_up_detected(void);
static void show_brightness_menu(void);
static void hide_brightness_menu(void);
static void update_brightness_display(void);
static void brightness_menu_close_cb(lv_event_t * e);
static void brightness_circle_click_cb(lv_event_t * e);
static void show_color_menu(void);
static void hide_color_menu(void);
static void update_color_selection(void);
static void color_menu_select_cb(lv_event_t * e);
static void color_menu_close_cb(lv_event_t * e);
static void color_circle_click_cb(lv_event_t * e);

static bool check_swipe_gesture(lv_event_code_t code);

// Counter variables
static int32_t counter_value = 0;
static lv_obj_t *counter_label = NULL;

// Timer variables
static lv_obj_t *timer_label = NULL;
static esp_timer_handle_t timer_handle = NULL;
static uint32_t timer_seconds = 3000; // 50 minutes in seconds
static bool timer_running = false;
static int64_t timer_start_time = 0;
static uint32_t timer_elapsed_total = 0; // Total elapsed time in seconds
static lv_obj_t *timer_touch_area = NULL;

// Timer setting popup variables
static lv_obj_t *timer_popup = NULL;
static lv_obj_t *timer_set_label = NULL;
static lv_obj_t *timer_arc = NULL;
static lv_obj_t *timer_circle = NULL;
static bool timer_setting_mode = false;
static uint32_t timer_setting_minutes = 50; // Default 50 minutes
static bool encoder_in_timer_mode = false;

// Timer alarm variables
static bool timer_alarm_active = false;
static lv_obj_t *alarm_message_label = NULL;
static esp_timer_handle_t alarm_timer_handle = NULL;

// Match counter variables
static lv_obj_t *match_circle_1 = NULL;
static lv_obj_t *match_circle_2 = NULL;
static lv_obj_t *match_circle_3 = NULL;
static lv_obj_t *match_touch_area = NULL;
static lv_obj_t *match_popup = NULL;
static bool match_popup_open = false;

// Match tracking data
static int match_me_wins = 0;
static int match_draws = 0;
static int match_opp_wins = 0;
static bool match_rounds[3] = {false, false, false}; // Track completed rounds
static int match_round_results[3] = {0, 0, 0}; // 0=none, 1=me, 2=draw, 3=opp

// Main menu variables
static lv_obj_t *main_menu = NULL;
static bool main_menu_open = false;
static int main_menu_selected = 0;  // Currently selected menu item (0-4)
static lv_obj_t *menu_items[5] = {NULL, NULL, NULL, NULL, NULL};
static lv_obj_t *menu_center_button = NULL;
static const char* menu_labels[] = {"Timer", "Match", "Brightness", "Colors", "Base"};
static lv_color_t menu_colors[] = {
    {.full = 0x07E0},    // Green for Timer (16-bit)
    {.full = 0xF800},    // Red for Match (16-bit)
    {.full = 0xFFE0},    // Yellow for Brightness (16-bit)
    {.full = 0xF81F},    // Magenta for Colors (16-bit)
    {.full = 0x07FF}     // Cyan for Base (16-bit)
};

// Brightness menu variables
static lv_obj_t *brightness_menu = NULL;
static bool brightness_menu_open = false;
static int brightness_level = 255;  // Current brightness (0-255)
static lv_obj_t *brightness_circle = NULL;
static lv_obj_t *brightness_arc = NULL;
static lv_obj_t *brightness_percentage_label = NULL;
static lv_obj_t *brightness_title_label = NULL;

// Color menu variables
static lv_obj_t *color_menu = NULL;
static bool color_menu_open = false;
static int selected_color_index = 0;  // Current selected color (0-5)
static lv_obj_t *color_bubbles[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
static lv_obj_t *color_center_button = NULL;
static lv_obj_t *color_title_label = NULL;
static const char* color_names[] = {"Dark Red", "Dark Green", "Orange", "Blue", "Light Olive", "Dark Purple"};
static lv_color_t color_values[] = {
    LV_COLOR_MAKE(0xbb, 0x1a, 0x22),  // #bb1a22 - Dark Red
    LV_COLOR_MAKE(0x06, 0x72, 0x43),  // #067243 - Dark Green  
    LV_COLOR_MAKE(0xcb, 0x7f, 0x24),  // #cb7f24 - Orange
    LV_COLOR_MAKE(0x38, 0x83, 0xc5),  // #3883c5 - Blue
    LV_COLOR_MAKE(0xec, 0xee, 0xdc),  // #eceedc - Light Olive
    LV_COLOR_MAKE(0x1c, 0x0f, 0x3b)   // #1c0f3b - Dark Purple
};

// Swipe detection variables
static bool touch_started = false;
static lv_coord_t touch_start_y = 0;
static lv_coord_t touch_start_x = 0;
static uint32_t touch_start_time = 0;

// Battery monitoring variables
static lv_obj_t *battery_icon = NULL;
static lv_obj_t *battery_percentage_label = NULL;
static lv_obj_t *battery_time_label = NULL;
static int battery_percentage = 85; // Mock value for now
static uint32_t battery_start_time = 0;
static int initial_battery = 100;


#if CONFIG_LV_COLOR_DEPTH == 32
#define LCD_BIT_PER_PIXEL       (24)
#elif CONFIG_LV_COLOR_DEPTH == 16
#define LCD_BIT_PER_PIXEL       (16)
#endif
//d5-d7
static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xF0, (uint8_t[]){0x28}, 1, 0},
    {0xF2, (uint8_t[]){0x28}, 1, 0},
    {0x73, (uint8_t[]){0xF0}, 1, 0},
    {0x7C, (uint8_t[]){0xD1}, 1, 0},
    {0x83, (uint8_t[]){0xE0}, 1, 0},
    {0x84, (uint8_t[]){0x61}, 1, 0},
    {0xF2, (uint8_t[]){0x82}, 1, 0},
    {0xF0, (uint8_t[]){0x00}, 1, 0},
    {0xF0, (uint8_t[]){0x01}, 1, 0},
    {0xF1, (uint8_t[]){0x01}, 1, 0},
    {0xB0, (uint8_t[]){0x56}, 1, 0},
    {0xB1, (uint8_t[]){0x4D}, 1, 0},
    {0xB2, (uint8_t[]){0x24}, 1, 0},
    {0xB4, (uint8_t[]){0x87}, 1, 0},
    {0xB5, (uint8_t[]){0x44}, 1, 0},
    {0xB6, (uint8_t[]){0x8B}, 1, 0},
    {0xB7, (uint8_t[]){0x40}, 1, 0},
    {0xB8, (uint8_t[]){0x86}, 1, 0},
    {0xBA, (uint8_t[]){0x00}, 1, 0},
    {0xBB, (uint8_t[]){0x08}, 1, 0},
    {0xBC, (uint8_t[]){0x08}, 1, 0},
    {0xBD, (uint8_t[]){0x00}, 1, 0},
    {0xC0, (uint8_t[]){0x80}, 1, 0},
    {0xC1, (uint8_t[]){0x10}, 1, 0},
    {0xC2, (uint8_t[]){0x37}, 1, 0},
    {0xC3, (uint8_t[]){0x80}, 1, 0},
    {0xC4, (uint8_t[]){0x10}, 1, 0},
    {0xC5, (uint8_t[]){0x37}, 1, 0},
    {0xC6, (uint8_t[]){0xA9}, 1, 0},
    {0xC7, (uint8_t[]){0x41}, 1, 0},
    {0xC8, (uint8_t[]){0x01}, 1, 0},
    {0xC9, (uint8_t[]){0xA9}, 1, 0},
    {0xCA, (uint8_t[]){0x41}, 1, 0},
    {0xCB, (uint8_t[]){0x01}, 1, 0},
    {0xD0, (uint8_t[]){0x91}, 1, 0},
    {0xD1, (uint8_t[]){0x68}, 1, 0},
    {0xD2, (uint8_t[]){0x68}, 1, 0},
    {0xF5, (uint8_t[]){0x00, 0xA5}, 2, 0},
    {0xDD, (uint8_t[]){0x4F}, 1, 0},
    {0xDE, (uint8_t[]){0x4F}, 1, 0},
    {0xF1, (uint8_t[]){0x10}, 1, 0},
    {0xF0, (uint8_t[]){0x00}, 1, 0},
    {0xF0, (uint8_t[]){0x02}, 1, 0},
    {0xE0, (uint8_t[]){0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33, 0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34}, 14, 0},
    {0xE1, (uint8_t[]){0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33, 0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33}, 14, 0},
    {0xF0, (uint8_t[]){0x10}, 1, 0},
    {0xF3, (uint8_t[]){0x10}, 1, 0},
    {0xE0, (uint8_t[]){0x07}, 1, 0},
    {0xE1, (uint8_t[]){0x00}, 1, 0},
    {0xE2, (uint8_t[]){0x00}, 1, 0},
    {0xE3, (uint8_t[]){0x00}, 1, 0},
    {0xE4, (uint8_t[]){0xE0}, 1, 0},
    {0xE5, (uint8_t[]){0x06}, 1, 0},
    {0xE6, (uint8_t[]){0x21}, 1, 0},
    {0xE7, (uint8_t[]){0x01}, 1, 0},
    {0xE8, (uint8_t[]){0x05}, 1, 0},
    {0xE9, (uint8_t[]){0x02}, 1, 0},
    {0xEA, (uint8_t[]){0xDA}, 1, 0},
    {0xEB, (uint8_t[]){0x00}, 1, 0},
    {0xEC, (uint8_t[]){0x00}, 1, 0},
    {0xED, (uint8_t[]){0x0F}, 1, 0},
    {0xEE, (uint8_t[]){0x00}, 1, 0},
    {0xEF, (uint8_t[]){0x00}, 1, 0},
    {0xF8, (uint8_t[]){0x00}, 1, 0},
    {0xF9, (uint8_t[]){0x00}, 1, 0},
    {0xFA, (uint8_t[]){0x00}, 1, 0},
    {0xFB, (uint8_t[]){0x00}, 1, 0},
    {0xFC, (uint8_t[]){0x00}, 1, 0},
    {0xFD, (uint8_t[]){0x00}, 1, 0},
    {0xFE, (uint8_t[]){0x00}, 1, 0},
    {0xFF, (uint8_t[]){0x00}, 1, 0},
    {0x60, (uint8_t[]){0x40}, 1, 0},
    {0x61, (uint8_t[]){0x04}, 1, 0},
    {0x62, (uint8_t[]){0x00}, 1, 0},
    {0x63, (uint8_t[]){0x42}, 1, 0},
    {0x64, (uint8_t[]){0xD9}, 1, 0},
    {0x65, (uint8_t[]){0x00}, 1, 0},
    {0x66, (uint8_t[]){0x00}, 1, 0},
    {0x67, (uint8_t[]){0x00}, 1, 0},
    {0x68, (uint8_t[]){0x00}, 1, 0},
    {0x69, (uint8_t[]){0x00}, 1, 0},
    {0x6A, (uint8_t[]){0x00}, 1, 0},
    {0x6B, (uint8_t[]){0x00}, 1, 0},
    {0x70, (uint8_t[]){0x40}, 1, 0},
    {0x71, (uint8_t[]){0x03}, 1, 0},
    {0x72, (uint8_t[]){0x00}, 1, 0},
    {0x73, (uint8_t[]){0x42}, 1, 0},
    {0x74, (uint8_t[]){0xD8}, 1, 0},
    {0x75, (uint8_t[]){0x00}, 1, 0},
    {0x76, (uint8_t[]){0x00}, 1, 0},
    {0x77, (uint8_t[]){0x00}, 1, 0},
    {0x78, (uint8_t[]){0x00}, 1, 0},
    {0x79, (uint8_t[]){0x00}, 1, 0},
    {0x7A, (uint8_t[]){0x00}, 1, 0},
    {0x7B, (uint8_t[]){0x00}, 1, 0},
    {0x80, (uint8_t[]){0x48}, 1, 0},
    {0x81, (uint8_t[]){0x00}, 1, 0},
    {0x82, (uint8_t[]){0x06}, 1, 0},
    {0x83, (uint8_t[]){0x02}, 1, 0},
    {0x84, (uint8_t[]){0xD6}, 1, 0},
    {0x85, (uint8_t[]){0x04}, 1, 0},
    {0x86, (uint8_t[]){0x00}, 1, 0},
    {0x87, (uint8_t[]){0x00}, 1, 0},
    {0x88, (uint8_t[]){0x48}, 1, 0},
    {0x89, (uint8_t[]){0x00}, 1, 0},
    {0x8A, (uint8_t[]){0x08}, 1, 0},
    {0x8B, (uint8_t[]){0x02}, 1, 0},
    {0x8C, (uint8_t[]){0xD8}, 1, 0},
    {0x8D, (uint8_t[]){0x04}, 1, 0},
    {0x8E, (uint8_t[]){0x00}, 1, 0},
    {0x8F, (uint8_t[]){0x00}, 1, 0},
    {0x90, (uint8_t[]){0x48}, 1, 0},
    {0x91, (uint8_t[]){0x00}, 1, 0},
    {0x92, (uint8_t[]){0x0A}, 1, 0},
    {0x93, (uint8_t[]){0x02}, 1, 0},
    {0x94, (uint8_t[]){0xDA}, 1, 0},
    {0x95, (uint8_t[]){0x04}, 1, 0},
    {0x96, (uint8_t[]){0x00}, 1, 0},
    {0x97, (uint8_t[]){0x00}, 1, 0},
    {0x98, (uint8_t[]){0x48}, 1, 0},
    {0x99, (uint8_t[]){0x00}, 1, 0},
    {0x9A, (uint8_t[]){0x0C}, 1, 0},
    {0x9B, (uint8_t[]){0x02}, 1, 0},
    {0x9C, (uint8_t[]){0xDC}, 1, 0},
    {0x9D, (uint8_t[]){0x04}, 1, 0},
    {0x9E, (uint8_t[]){0x00}, 1, 0},
    {0x9F, (uint8_t[]){0x00}, 1, 0},
    {0xA0, (uint8_t[]){0x48}, 1, 0},
    {0xA1, (uint8_t[]){0x00}, 1, 0},
    {0xA2, (uint8_t[]){0x05}, 1, 0},
    {0xA3, (uint8_t[]){0x02}, 1, 0},
    {0xA4, (uint8_t[]){0xD5}, 1, 0},
    {0xA5, (uint8_t[]){0x04}, 1, 0},
    {0xA6, (uint8_t[]){0x00}, 1, 0},
    {0xA7, (uint8_t[]){0x00}, 1, 0},
    {0xA8, (uint8_t[]){0x48}, 1, 0},
    {0xA9, (uint8_t[]){0x00}, 1, 0},
    {0xAA, (uint8_t[]){0x07}, 1, 0},
    {0xAB, (uint8_t[]){0x02}, 1, 0},
    {0xAC, (uint8_t[]){0xD7}, 1, 0},
    {0xAD, (uint8_t[]){0x04}, 1, 0},
    {0xAE, (uint8_t[]){0x00}, 1, 0},
    {0xAF, (uint8_t[]){0x00}, 1, 0},
    {0xB0, (uint8_t[]){0x48}, 1, 0},
    {0xB1, (uint8_t[]){0x00}, 1, 0},
    {0xB2, (uint8_t[]){0x09}, 1, 0},
    {0xB3, (uint8_t[]){0x02}, 1, 0},
    {0xB4, (uint8_t[]){0xD9}, 1, 0},
    {0xB5, (uint8_t[]){0x04}, 1, 0},
    {0xB6, (uint8_t[]){0x00}, 1, 0},
    {0xB7, (uint8_t[]){0x00}, 1, 0},
    {0xB8, (uint8_t[]){0x48}, 1, 0},
    {0xB9, (uint8_t[]){0x00}, 1, 0},
    {0xBA, (uint8_t[]){0x0B}, 1, 0},
    {0xBB, (uint8_t[]){0x02}, 1, 0},
    {0xBC, (uint8_t[]){0xDB}, 1, 0},
    {0xBD, (uint8_t[]){0x04}, 1, 0},
    {0xBE, (uint8_t[]){0x00}, 1, 0},
    {0xBF, (uint8_t[]){0x00}, 1, 0},
    {0xC0, (uint8_t[]){0x10}, 1, 0},
    {0xC1, (uint8_t[]){0x47}, 1, 0},
    {0xC2, (uint8_t[]){0x56}, 1, 0},
    {0xC3, (uint8_t[]){0x65}, 1, 0},
    {0xC4, (uint8_t[]){0x74}, 1, 0},
    {0xC5, (uint8_t[]){0x88}, 1, 0},
    {0xC6, (uint8_t[]){0x99}, 1, 0},
    {0xC7, (uint8_t[]){0x01}, 1, 0},
    {0xC8, (uint8_t[]){0xBB}, 1, 0},
    {0xC9, (uint8_t[]){0xAA}, 1, 0},
    {0xD0, (uint8_t[]){0x10}, 1, 0},
    {0xD1, (uint8_t[]){0x47}, 1, 0},
    {0xD2, (uint8_t[]){0x56}, 1, 0},
    {0xD3, (uint8_t[]){0x65}, 1, 0},
    {0xD4, (uint8_t[]){0x74}, 1, 0},
    {0xD5, (uint8_t[]){0x88}, 1, 0},
    {0xD6, (uint8_t[]){0x99}, 1, 0},
    {0xD7, (uint8_t[]){0x01}, 1, 0},
    {0xD8, (uint8_t[]){0xBB}, 1, 0},
    {0xD9, (uint8_t[]){0xAA}, 1, 0},
    {0xF3, (uint8_t[]){0x01}, 1, 0},
    {0xF0, (uint8_t[]){0x00}, 1, 0},
    {0x21, (uint8_t[]){0x00}, 1, 0},
    {0x11, (uint8_t[]){0x00}, 1, 120},
    {0x29, (uint8_t[]){0x00}, 1, 0},
#ifdef EXAMPLE_Rotate_90
    {0x36, (uint8_t[]){0x60}, 1, 0},
#else
    {0x36, (uint8_t[]){0x00}, 1, 0},
#endif
};

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    const int offsetx1 = area->x1;
    const int offsetx2 = area->x2;
    const int offsety1 = area->y1;
    const int offsety2 = area->y2;

#if LCD_BIT_PER_PIXEL == 24
    uint8_t *to = (uint8_t *)color_map;
    uint8_t temp = 0;
    uint16_t pixel_num = (offsetx2 - offsetx1 + 1) * (offsety2 - offsety1 + 1);

    // Special dealing for first pixel
    temp = color_map[0].ch.blue;
    *to++ = color_map[0].ch.red;
    *to++ = color_map[0].ch.green;
    *to++ = temp;
    // Normal dealing for other pixels
    for (int i = 1; i < pixel_num; i++) {
        *to++ = color_map[i].ch.red;
        *to++ = color_map[i].ch.green;
        *to++ = color_map[i].ch.blue;
    }
#endif

    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

void example_lvgl_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    uint16_t x1 = area->x1;
    uint16_t x2 = area->x2;

    uint16_t y1 = area->y1;
    uint16_t y2 = area->y2;

    // round the start of coordinate down to the nearest 2M number
    area->x1 = (x1 >> 1) << 1;
    area->y1 = (y1 >> 1) << 1;
    // round the end of coordinate up to the nearest 2N+1 number
    area->x2 = ((x2 >> 1) << 1) + 1;
    area->y2 = ((y2 >> 1) << 1) + 1;
}

#if EXAMPLE_USE_TOUCH
static void example_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t tp_x;
    uint16_t tp_y;
    uint8_t win = tpGetCoordinates(&tp_x,&tp_y);
    if (win)
    {
        #ifdef EXAMPLE_Rotate_90
            data->point.x = tp_y;
            data->point.y = (EXAMPLE_LCD_V_RES - tp_x);
        #else
            data->point.x = tp_x;
            data->point.y = tp_y;
        #endif
        if(data->point.x > EXAMPLE_LCD_H_RES)
        data->point.x = EXAMPLE_LCD_H_RES;
        if(data->point.y > EXAMPLE_LCD_V_RES)
        data->point.y = EXAMPLE_LCD_V_RES;
        data->state = LV_INDEV_STATE_PRESSED;
        //ESP_LOGE("TP","(%d,%d)",data->point.x,data->point.y);
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
#endif

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static bool example_lvgl_lock(int timeout_ms)
{
    assert(lvgl_mux && "bsp_display_start must be called first");

    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

static void example_lvgl_unlock(void)
{
    assert(lvgl_mux && "bsp_display_start must be called first");
    xSemaphoreGive(lvgl_mux);
}

static void counter_long_press_cb(lv_event_t * e)
{
    ESP_LOGI(TAG, "Counter reset by long press");
    counter_value = 0;
    
    // Update display
    if (counter_label != NULL) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%ld", counter_value);
        lv_label_set_text(counter_label, buf);
    }
}




static void timer_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    // Check for swipe gesture first
    if (check_swipe_gesture(code)) {
        return; // Swipe was detected, don't process as click
    }
    
    if (code == LV_EVENT_CLICKED) {
        // Don't allow timer toggle when alarm is active
        if (timer_alarm_active) {
            ESP_LOGI(TAG, "Timer click ignored - alarm is active");
            return;
        }
        
        // Toggle timer start/pause
        timer_running = !timer_running;
        
        if (timer_running) {
            timer_start_time = esp_timer_get_time();
            ESP_LOGI(TAG, "Timer started");
        } else {
            // Add elapsed time to total when pausing
            if (timer_start_time > 0) {
                int64_t elapsed_us = esp_timer_get_time() - timer_start_time;
                timer_elapsed_total += (uint32_t)(elapsed_us / 1000000); // Convert to seconds
            }
            ESP_LOGI(TAG, "Timer paused");
        }
    }
    // Long press functionality removed - use main menu instead
}


static bool check_swipe_gesture(lv_event_code_t code)
{
    // Only allow swipe gestures on the main screen (no menus open)
    if (main_menu_open || brightness_menu_open || color_menu_open || timer_setting_mode) {
        return false;
    }
    
    if (code == LV_EVENT_PRESSED) {
        // Touch started - record position and time
        lv_indev_t * indev = lv_indev_get_act();
        if (indev != NULL) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            
            touch_started = true;
            touch_start_x = point.x;
            touch_start_y = point.y;
            touch_start_time = lv_tick_get();
            
            ESP_LOGI(TAG, "Touch started at (%d, %d)", touch_start_x, touch_start_y);
        }
        return false; // Don't consume the event
    } else if (code == LV_EVENT_RELEASED && touch_started) {
        // Touch ended - check for swipe
        lv_indev_t * indev = lv_indev_get_act();
        if (indev != NULL) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            
            lv_coord_t dx = point.x - touch_start_x;
            lv_coord_t dy = point.y - touch_start_y;
            uint32_t duration = lv_tick_get() - touch_start_time;
            
            ESP_LOGI(TAG, "Touch ended at (%d, %d), delta: (%d, %d), duration: %lu ms", 
                     point.x, point.y, dx, dy, duration);
            
            // Check for upward swipe: 
            // - Must be predominantly vertical (|dy| > |dx|)
            // - Must be upward (dy < 0, negative because y=0 is at top)
            // - Must have enough distance (at least 50 pixels)
            // - Must not be too slow (less than 1000ms)
            if (abs(dy) > abs(dx) && dy < -50 && duration < 1000) {
                ESP_LOGI(TAG, "Upward swipe detected!");
                swipe_up_detected();
                touch_started = false;
                return true; // Consume the event - don't process as click
            }
        }
        touch_started = false;
    }
    return false; // Don't consume the event
}

static lv_color_t get_battery_color(int percentage)
{
    if (percentage >= 30) {
        return lv_color_hex(0x00AA00); // Green
    } else if (percentage >= 10) {
        return lv_color_hex(0xFFAA00); // Yellow/Orange
    } else {
        return lv_color_hex(0xFF0000); // Red
    }
}

static void create_battery_indicator(lv_obj_t *parent)
{
    // Create battery icon container - positioned at top of wheel
    battery_icon = lv_obj_create(parent);
    lv_obj_set_size(battery_icon, 70, 35);
    lv_obj_align(battery_icon, LV_ALIGN_TOP_MID, 0, 30); // Top center, aligned with other wheel items
    lv_obj_set_style_radius(battery_icon, 6, LV_PART_MAIN);
    lv_obj_set_style_border_width(battery_icon, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(battery_icon, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(battery_icon, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // Create battery tip (small rectangle on the right side)
    lv_obj_t *battery_tip = lv_obj_create(battery_icon);
    lv_obj_set_size(battery_tip, 4, 14);
    lv_obj_align(battery_tip, LV_ALIGN_RIGHT_MID, 6, 0);
    lv_obj_set_style_radius(battery_tip, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(battery_tip, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(battery_tip, 0, LV_PART_MAIN);
    
    // Create battery fill (will be sized based on percentage)
    lv_obj_t *battery_fill = lv_obj_create(battery_icon);
    lv_obj_set_style_radius(battery_fill, 3, LV_PART_MAIN);
    lv_obj_set_style_border_width(battery_fill, 0, LV_PART_MAIN);
    lv_obj_align(battery_fill, LV_ALIGN_LEFT_MID, 2, 0);
    
    // Set initial fill based on battery percentage
    int fill_width = (int)((battery_percentage / 100.0) * 62); // 62 = 70-8 for padding
    lv_obj_set_size(battery_fill, fill_width, 27);
    lv_obj_set_style_bg_color(battery_fill, get_battery_color(battery_percentage), LV_PART_MAIN);
    
    // Create percentage label inside battery
    battery_percentage_label = lv_label_create(battery_icon);
    char percentage_text[8];
    snprintf(percentage_text, sizeof(percentage_text), "%d%%", battery_percentage);
    lv_label_set_text(battery_percentage_label, percentage_text);
    lv_obj_set_style_text_color(battery_percentage_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(battery_percentage_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(battery_percentage_label);
    
    // Create time remaining label below battery (smaller, more compact)
    battery_time_label = lv_label_create(parent);
    lv_obj_set_style_text_color(battery_time_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(battery_time_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(battery_time_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(battery_time_label, LV_ALIGN_TOP_MID, 0, 70); // Just below battery icon
    
    update_battery_display();
}

static void update_battery_display(void)
{
    if (battery_icon == NULL || battery_percentage_label == NULL || battery_time_label == NULL) {
        return;
    }
    
    // Simulate battery drain (decrease by 1% every 2 minutes for demo purposes)
    static uint32_t last_battery_update = 0;
    uint32_t current_time = esp_timer_get_time() / 1000000; // Convert to seconds
    
    if (last_battery_update == 0) {
        last_battery_update = current_time;
    } else if (current_time - last_battery_update > 120) { // Every 2 minutes
        if (battery_percentage > 0) {
            battery_percentage--;
            last_battery_update = current_time;
            ESP_LOGI(TAG, "Battery simulation: %d%%", battery_percentage);
        }
    }
    
    // Update percentage text
    char percentage_text[8];
    snprintf(percentage_text, sizeof(percentage_text), "%d%%", battery_percentage);
    lv_label_set_text(battery_percentage_label, percentage_text);
    
    // Update battery fill
    lv_obj_t *battery_fill = lv_obj_get_child(battery_icon, 1); // Second child (after tip)
    if (battery_fill != NULL) {
        int fill_width = (int)((battery_percentage / 100.0) * 62); // 62 = 70-8 for padding
        lv_obj_set_size(battery_fill, fill_width, 27);
        lv_obj_set_style_bg_color(battery_fill, get_battery_color(battery_percentage), LV_PART_MAIN);
    }
    
    // Calculate estimated time remaining
    if (battery_start_time == 0) {
        // Initialize battery monitoring
        battery_start_time = esp_timer_get_time() / 1000000; // Convert to seconds
        initial_battery = battery_percentage;
    }
    
    uint32_t elapsed_seconds = (esp_timer_get_time() / 1000000) - battery_start_time;
    
    char time_text[32];
    if (elapsed_seconds > 60 && initial_battery > battery_percentage) {
        // Calculate drain rate
        int battery_used = initial_battery - battery_percentage;
        float drain_rate = (float)battery_used / (elapsed_seconds / 3600.0); // % per hour
        
        if (drain_rate > 0) {
            float hours_remaining = battery_percentage / drain_rate;
            int hours = (int)hours_remaining;
            int minutes = (int)((hours_remaining - hours) * 60);
            
            if (hours > 0) {
                snprintf(time_text, sizeof(time_text), "%dh %dm left", hours, minutes);
            } else {
                snprintf(time_text, sizeof(time_text), "%dm left", minutes);
            }
        } else {
            snprintf(time_text, sizeof(time_text), "-- left");
        }
    } else {
        snprintf(time_text, sizeof(time_text), "Calculating...");
    }
    
    lv_label_set_text(battery_time_label, time_text);
}

// Brightness menu functions
static void show_brightness_menu(void)
{
    if (brightness_menu_open) return; // Prevent multiple menus
    
    brightness_menu_open = true;
    
    // Create brightness menu container
    brightness_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(brightness_menu, 360, 360);
    lv_obj_center(brightness_menu);
    lv_obj_set_style_bg_color(brightness_menu, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(brightness_menu, 220, LV_PART_MAIN); // Semi-transparent
    lv_obj_set_style_border_opa(brightness_menu, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(brightness_menu, 20, LV_PART_MAIN);
    lv_obj_clear_flag(brightness_menu, LV_OBJ_FLAG_SCROLLABLE);
    // Remove background click event - only circle should be clickable
    
    // Create circular background - same size as Match menu (320x320)
    brightness_circle = lv_obj_create(brightness_menu);
    lv_obj_set_size(brightness_circle, 320, 320);
    lv_obj_center(brightness_circle);
    lv_obj_set_style_radius(brightness_circle, 160, LV_PART_MAIN);
    lv_obj_set_style_bg_color(brightness_circle, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_border_color(brightness_circle, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_border_width(brightness_circle, 2, LV_PART_MAIN);
    lv_obj_clear_flag(brightness_circle, LV_OBJ_FLAG_SCROLLABLE);
    // Make only the circle clickable for tap-to-close
    lv_obj_add_event_cb(brightness_circle, brightness_circle_click_cb, LV_EVENT_CLICKED, NULL);
    
    // Create arc for brightness level visualization - same size as Match menu (296x296)
    brightness_arc = lv_arc_create(brightness_menu);
    lv_obj_set_size(brightness_arc, 296, 296);
    lv_obj_center(brightness_arc);
    lv_arc_set_range(brightness_arc, 0, 255);
    lv_arc_set_value(brightness_arc, brightness_level);
    lv_arc_set_bg_angles(brightness_arc, 0, 360);
    lv_obj_set_style_arc_color(brightness_arc, lv_color_hex(0xFFE0), LV_PART_INDICATOR); // Yellow
    lv_obj_set_style_arc_color(brightness_arc, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_width(brightness_arc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(brightness_arc, 8, LV_PART_MAIN);
    lv_obj_remove_style(brightness_arc, NULL, LV_PART_KNOB); // Hide the knob
    lv_obj_clear_flag(brightness_arc, LV_OBJ_FLAG_CLICKABLE);
    
    // Create percentage label in center
    brightness_percentage_label = lv_label_create(brightness_menu);
    lv_obj_set_style_text_color(brightness_percentage_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(brightness_percentage_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_align(brightness_percentage_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_center(brightness_percentage_label);
    
    // Remove instruction text as requested
    
    update_brightness_display();
    
    ESP_LOGI(TAG, "Brightness menu opened");
}

static void hide_brightness_menu(void)
{
    if (brightness_menu != NULL) {
        lv_obj_del(brightness_menu);
        brightness_menu = NULL;
        brightness_circle = NULL;
        brightness_arc = NULL;
        brightness_percentage_label = NULL;
    }
    brightness_menu_open = false;
    ESP_LOGI(TAG, "Brightness menu closed");
}

static void update_brightness_display(void)
{
    if (brightness_percentage_label != NULL) {
        int percentage = (brightness_level * 100) / 255;
        char text[8];
        snprintf(text, sizeof(text), "%d%%", percentage);
        lv_label_set_text(brightness_percentage_label, text);
    }
    
    if (brightness_arc != NULL) {
        lv_arc_set_value(brightness_arc, brightness_level);
    }
    
    // Update actual screen brightness
    setUpduty(brightness_level);
}

static void brightness_menu_close_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED) {
        ESP_LOGI(TAG, "Brightness menu close requested");
        haptic_feedback_short();
        hide_brightness_menu();
    }
}

static void brightness_circle_click_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Brightness circle clicked - closing menu");
        haptic_feedback_short();
        hide_brightness_menu();
    }
}

// Color menu functions
static void show_color_menu(void)
{
    if (color_menu_open) return; // Prevent multiple menus
    
    color_menu_open = true;
    
    // Create color menu container - match brightness menu style
    color_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(color_menu, 360, 360);
    lv_obj_center(color_menu);
    lv_obj_set_style_bg_color(color_menu, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(color_menu, 220, LV_PART_MAIN); // Semi-transparent
    lv_obj_set_style_border_opa(color_menu, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(color_menu, 20, LV_PART_MAIN);
    lv_obj_clear_flag(color_menu, LV_OBJ_FLAG_SCROLLABLE);
    // Remove background click event - only circle should be clickable
    
    // Create circular background - same size as Match menu (320x320)
    lv_obj_t *color_circle = lv_obj_create(color_menu);
    lv_obj_set_size(color_circle, 320, 320);
    lv_obj_center(color_circle);
    lv_obj_set_style_radius(color_circle, 160, LV_PART_MAIN);
    lv_obj_set_style_bg_color(color_circle, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_border_color(color_circle, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_border_width(color_circle, 2, LV_PART_MAIN);
    lv_obj_clear_flag(color_circle, LV_OBJ_FLAG_SCROLLABLE);
    // Make only the circle clickable for tap-to-close
    lv_obj_add_event_cb(color_circle, color_circle_click_cb, LV_EVENT_CLICKED, NULL);
    
    // Create center select button
    color_center_button = lv_btn_create(color_menu);
    lv_obj_set_size(color_center_button, 80, 80);
    lv_obj_center(color_center_button);
    lv_obj_set_style_bg_color(color_center_button, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_radius(color_center_button, 40, LV_PART_MAIN); // Circular
    lv_obj_add_event_cb(color_center_button, color_menu_select_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *select_label = lv_label_create(color_center_button);
    lv_label_set_text(select_label, "SELECT");
    lv_obj_set_style_text_color(select_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(select_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(select_label);
    
    // Create a single background arc for the full circle
    lv_obj_t *background_arc = lv_arc_create(color_menu);
    lv_obj_set_size(background_arc, 296, 296);
    lv_obj_center(background_arc);
    lv_arc_set_range(background_arc, 0, 360);
    lv_arc_set_value(background_arc, 360);
    lv_arc_set_bg_angles(background_arc, 0, 360);
    lv_arc_set_angles(background_arc, 0, 360);
    lv_obj_set_style_arc_color(background_arc, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_arc_color(background_arc, lv_color_hex(0x222222), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(background_arc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(background_arc, 8, LV_PART_INDICATOR);
    lv_obj_remove_style(background_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(background_arc, LV_OBJ_FLAG_CLICKABLE);
    
    // Create individual color arcs with double-sized gaps
    // 360 degrees / 6 colors = 60 degrees per section
    // Use 50 degrees for color, 10 degrees for gap (doubled from 5)
    for (int i = 0; i < 6; i++) {
        lv_obj_t *color_arc = lv_arc_create(color_menu);
        lv_obj_set_size(color_arc, 296, 296);
        lv_obj_center(color_arc);
        
        // Calculate angles with double-sized gaps: 50° segments with 10° gaps
        int start_angle = (i * 60) + 5; // Start 5° into each 60° section
        int end_angle = start_angle + 50;   // 50° segment length
        
        // Configure the arc to only show the colored segment
        lv_arc_set_range(color_arc, 0, 100);
        lv_arc_set_value(color_arc, 100);
        lv_arc_set_bg_angles(color_arc, start_angle, end_angle);
        lv_arc_set_angles(color_arc, start_angle, end_angle);
        
        // Style the arc
        lv_obj_set_style_arc_color(color_arc, lv_color_hex(0x000000), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(color_arc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_color(color_arc, color_values[i], LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(color_arc, 8, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(color_arc, 8, LV_PART_MAIN);
        
        // Remove the knob
        lv_obj_remove_style(color_arc, NULL, LV_PART_KNOB);
        lv_obj_clear_flag(color_arc, LV_OBJ_FLAG_CLICKABLE);
        
        // Store reference
        color_bubbles[i] = color_arc;
    }
    
    update_color_selection();
    
    ESP_LOGI(TAG, "Color menu opened");
}

static void hide_color_menu(void)
{
    if (color_menu != NULL) {
        lv_obj_del(color_menu);
        color_menu = NULL;
        color_center_button = NULL;
        color_title_label = NULL;
        
        // Clear color segment references
        for (int i = 0; i < 6; i++) {
            color_bubbles[i] = NULL;
        }
    }
    color_menu_open = false;
    ESP_LOGI(TAG, "Color menu closed");
}

static void update_color_selection(void)
{
    for (int i = 0; i < 6; i++) {
        if (color_bubbles[i] != NULL) {
            if (i == selected_color_index) {
                // Highlight selected color arc with thicker width
                lv_obj_set_style_arc_width(color_bubbles[i], 16, LV_PART_INDICATOR); // Thicker
                lv_obj_set_style_arc_width(color_bubbles[i], 16, LV_PART_MAIN);
                lv_obj_set_style_shadow_color(color_bubbles[i], lv_color_white(), LV_PART_MAIN);
                lv_obj_set_style_shadow_width(color_bubbles[i], 8, LV_PART_MAIN);
                lv_obj_set_style_shadow_opa(color_bubbles[i], LV_OPA_50, LV_PART_MAIN);
            } else {
                // Normal appearance
                lv_obj_set_style_arc_width(color_bubbles[i], 8, LV_PART_INDICATOR); // Normal thickness
                lv_obj_set_style_arc_width(color_bubbles[i], 8, LV_PART_MAIN);
                lv_obj_set_style_shadow_width(color_bubbles[i], 0, LV_PART_MAIN);
                lv_obj_set_style_shadow_opa(color_bubbles[i], LV_OPA_TRANSP, LV_PART_MAIN);
            }
        }
    }
}

static lv_color_t get_contrasting_text_color(int color_index)
{
    // Determine if the background is light or dark and return contrasting text color
    switch (color_index) {
        case 4: // LIGHT OLIVE - use black text (very light color)
            return lv_color_black();
        case 0: // DARK RED - use white text
        case 1: // DARK GREEN - use white text
        case 2: // ORANGE - use white text
        case 3: // BLUE - use white text
        case 5: // DARK PURPLE - use white text
        default:
            return lv_color_white();
    }
}

static void update_text_colors(int color_index)
{
    lv_color_t text_color = get_contrasting_text_color(color_index);
    
    // Update all text elements on the main screen
    if (timer_label != NULL) {
        lv_obj_set_style_text_color(timer_label, text_color, LV_PART_MAIN);
    }
    if (counter_label != NULL) {
        lv_obj_set_style_text_color(counter_label, text_color, LV_PART_MAIN);
    }
    if (alarm_message_label != NULL) {
        lv_obj_set_style_text_color(alarm_message_label, lv_color_hex(0xFF0000), LV_PART_MAIN); // Keep alarm red
    }
    
    // Match counter circles don't need text color updates since they use background colors
    // The circles will maintain their result colors based on game outcomes
}

static void apply_background_color(int color_index)
{
    if (color_index >= 0 && color_index < 6) {
        lv_obj_t *scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, color_values[color_index], LV_PART_MAIN);
        
        // Update text colors to contrast with the new background
        update_text_colors(color_index);
        
        ESP_LOGI(TAG, "Background color changed to: %s with contrasting text", color_names[color_index]);
    }
}

static void color_menu_select_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Color applied: %s", color_names[selected_color_index]);
        
        // Apply the selected color to the background
        apply_background_color(selected_color_index);
        
        haptic_feedback_short();
        hide_color_menu();
    }
}

static void color_menu_close_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED) {
        ESP_LOGI(TAG, "Color menu close requested");
        haptic_feedback_short();
        hide_color_menu();
    }
}

static void color_circle_click_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Color circle clicked - closing menu");
        haptic_feedback_short();
        hide_color_menu();
    }
}

static void update_timer_display(void)
{
    if (timer_label != NULL) {
        uint32_t remaining_seconds;
        
        // If alarm is active, always show 00:00
        if (timer_alarm_active) {
            remaining_seconds = 0;
        } else {
            uint32_t total_elapsed = timer_elapsed_total;
            
            // Add current session elapsed time if timer is running
            if (timer_running && timer_start_time > 0) {
                int64_t elapsed_us = esp_timer_get_time() - timer_start_time;
                total_elapsed += (uint32_t)(elapsed_us / 1000000); // Convert to seconds
            }
            
            // Calculate remaining time (countdown from timer_seconds)
            if (total_elapsed >= timer_seconds) {
                remaining_seconds = 0; // Timer finished
                if (timer_running) {
                    timer_running = false; // Auto-stop when timer reaches 0
                    ESP_LOGI(TAG, "Timer finished!");
                    // Trigger alarm when timer finishes
                    start_timer_alarm();
                }
            } else {
                remaining_seconds = timer_seconds - total_elapsed;
            }
        }
        
        uint32_t minutes = remaining_seconds / 60;
        uint32_t seconds = remaining_seconds % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02lu:%02lu", minutes, seconds);
        lv_label_set_text(timer_label, buf);
    }
}

static void popup_close_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Timer setting popup closed - applying new time: %lu minutes", timer_setting_minutes);
        
        // Clear any active alarm when setting new timer
        stop_timer_alarm();
        
        // Apply the new timer setting
        timer_seconds = timer_setting_minutes * 60;
        timer_start_time = 0;
        timer_elapsed_total = 0;
        
        // Hide popup and exit timer setting mode
        hide_timer_setting_popup();
        timer_setting_mode = false;
        encoder_in_timer_mode = false;
        
        // Update timer display
        update_timer_display();
        
        haptic_feedback_short();
    }
}

static void show_timer_setting_popup(void)
{
    timer_setting_mode = true;
    encoder_in_timer_mode = true;
    timer_setting_minutes = timer_seconds / 60; // Set current timer value
    
    // Create timer menu container - match brightness menu style
    timer_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(timer_popup, 360, 360);
    lv_obj_center(timer_popup);
    lv_obj_set_style_bg_color(timer_popup, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(timer_popup, 220, LV_PART_MAIN); // Semi-transparent
    lv_obj_set_style_border_opa(timer_popup, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(timer_popup, 20, LV_PART_MAIN);
    lv_obj_clear_flag(timer_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(timer_popup, popup_close_cb, LV_EVENT_ALL, NULL);
    
    // Create circular background - same size as Match menu (320x320)
    timer_circle = lv_obj_create(timer_popup);
    lv_obj_set_size(timer_circle, 320, 320);
    lv_obj_center(timer_circle);
    lv_obj_set_style_radius(timer_circle, 160, LV_PART_MAIN);
    lv_obj_set_style_bg_color(timer_circle, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_border_color(timer_circle, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_set_style_border_width(timer_circle, 2, LV_PART_MAIN);
    lv_obj_clear_flag(timer_circle, LV_OBJ_FLAG_SCROLLABLE);
    // Make the circle clickable for tap-to-close
    lv_obj_add_event_cb(timer_circle, popup_close_cb, LV_EVENT_CLICKED, NULL);
    
    // Create arc for timer visualization - same size as Match menu (296x296)
    timer_arc = lv_arc_create(timer_popup);
    lv_obj_set_size(timer_arc, 296, 296);
    lv_obj_center(timer_arc);
    lv_arc_set_range(timer_arc, 1, 60); // 1 to 60 minutes max
    lv_arc_set_value(timer_arc, timer_setting_minutes);
    lv_arc_set_bg_angles(timer_arc, 0, 360);
    lv_obj_set_style_arc_color(timer_arc, lv_color_hex(0x00FF00), LV_PART_INDICATOR); // Green
    lv_obj_set_style_arc_color(timer_arc, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_arc_width(timer_arc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(timer_arc, 8, LV_PART_MAIN);
    lv_obj_remove_style(timer_arc, NULL, LV_PART_KNOB); // Hide the knob
    lv_obj_clear_flag(timer_arc, LV_OBJ_FLAG_CLICKABLE);
    
    // Create timer setting display label in the center
    timer_set_label = lv_label_create(timer_popup);
    lv_obj_set_style_text_color(timer_set_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(timer_set_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_align(timer_set_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_center(timer_set_label);
    update_timer_setting_display();
    
    // Remove instruction text as requested
    
    // Make the timer popup clickable for tap to close (instead of long press)
    lv_obj_add_flag(timer_popup, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(timer_popup, popup_close_cb, LV_EVENT_CLICKED, NULL);
}

static void hide_timer_setting_popup(void)
{
    if (timer_popup != NULL) {
        lv_obj_del(timer_popup);
        timer_popup = NULL;
        timer_set_label = NULL;
        timer_arc = NULL;
        timer_circle = NULL;
    }
}

static void update_timer_setting_display(void)
{
    if (timer_set_label != NULL) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02u:00", timer_setting_minutes);
        lv_label_set_text(timer_set_label, buf);
    }
    
    // Update the arc dynamically
    if (timer_arc != NULL) {
        lv_arc_set_value(timer_arc, timer_setting_minutes);
    }
}

// Haptic feedback function
static void haptic_feedback_short(void)
{
    // Use effect 10 - Double Click 100% (very strong effect)
    ESP_LOGI(TAG, "Triggering DOUBLE CLICK 100%% haptic feedback...");
    i2c_drv2605_haptic_feedback(10); 
    ESP_LOGI(TAG, "Double click haptic feedback call completed");
}

// Timer alarm haptic callback function
static void alarm_haptic_callback(void* arg)
{
    if (timer_alarm_active) {
        // Use the strongest alarm effect (effect 10 - Double Click 100% - very strong)
        ESP_LOGI(TAG, "Timer alarm STRONG haptic pulse");
        i2c_drv2605_haptic_feedback(10);
    }
}

// Timer alarm stop callback function
static void alarm_stop_callback(void* arg)
{
    ESP_LOGI(TAG, "Auto-stopping STRONG alarm haptics after 3 seconds (visual alarm remains)");
    stop_timer_alarm_haptics_only();
}

// Start timer alarm with haptic feedback
static void start_timer_alarm(void)
{
    ESP_LOGI(TAG, "Starting timer alarm!");
    timer_alarm_active = true;
    
    // Change timer text color to red
    if (timer_label != NULL) {
        lv_obj_set_style_text_color(timer_label, lv_color_hex(0xFF0000), LV_PART_MAIN);
    }
    
    // Show "Round Ended" message
    if (alarm_message_label != NULL) {
        lv_obj_clear_flag(alarm_message_label, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Start haptic alarm pattern - pulse every 100ms for 3 seconds (30 pulses)
    esp_timer_create_args_t alarm_timer_args = {
        .callback = alarm_haptic_callback,
        .arg = NULL,
        .name = "alarm_timer"
    };
    
    esp_err_t ret = esp_timer_create(&alarm_timer_args, &alarm_timer_handle);
    if (ret == ESP_OK) {
        // Start periodic timer (100ms intervals for 3 seconds = 30 strong pulses)
        esp_timer_start_periodic(alarm_timer_handle, 100000); // 100ms in microseconds
        ESP_LOGI(TAG, "STRONG alarm haptic timer started - will pulse for 3 seconds");
        
        // Create a one-shot timer to stop the alarm after 3 seconds
        esp_timer_handle_t stop_timer_handle;
        esp_timer_create_args_t stop_timer_args = {
            .callback = alarm_stop_callback,
            .arg = NULL,
            .name = "alarm_stop_timer"
        };
        
        esp_err_t stop_ret = esp_timer_create(&stop_timer_args, &stop_timer_handle);
        if (stop_ret == ESP_OK) {
            esp_timer_start_once(stop_timer_handle, 3000000); // 3 seconds
        }
    } else {
        ESP_LOGE(TAG, "Failed to create alarm timer: %s", esp_err_to_name(ret));
    }
}

// Stop only the haptic feedback, keep visual alarm active
static void stop_timer_alarm_haptics_only(void)
{
    ESP_LOGI(TAG, "Stopping timer alarm haptics only (keeping visual alarm)");
    
    // Stop haptic timer but keep alarm visually active
    if (alarm_timer_handle != NULL) {
        esp_timer_stop(alarm_timer_handle);
        esp_timer_delete(alarm_timer_handle);
        alarm_timer_handle = NULL;
    }
    // Note: timer_alarm_active remains true, visual elements stay red/visible
}

// Stop timer alarm completely (haptics + visual)
static void stop_timer_alarm(void)
{
    ESP_LOGI(TAG, "Stopping timer alarm");
    timer_alarm_active = false;
    
    // Stop haptic timer
    if (alarm_timer_handle != NULL) {
        esp_timer_stop(alarm_timer_handle);
        esp_timer_delete(alarm_timer_handle);
        alarm_timer_handle = NULL;
    }
    
    // Reset timer text color to white
    if (timer_label != NULL) {
        lv_obj_set_style_text_color(timer_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    }
    
    // Hide "Round Ended" message
    if (alarm_message_label != NULL) {
        lv_obj_add_flag(alarm_message_label, LV_OBJ_FLAG_HIDDEN);
    }
}

// Match counter functions
static void update_match_display(void)
{
    // Update game result circles based on match_round_results
    lv_obj_t* circles[3] = {match_circle_1, match_circle_2, match_circle_3};
    
    for (int i = 0; i < 3; i++) {
        if (circles[i] != NULL) {
            lv_color_t color;
            switch (match_round_results[i]) {
                case 0: // None/default
                    color = lv_color_hex(0x666666); // Gray
                    break;
                case 1: // Me wins
                    color = lv_color_hex(0x00AA00); // Green
                    break;
                case 2: // Draw
                    color = lv_color_hex(0xFFAA00); // Yellow/Orange
                    break;
                case 3: // Opponent wins
                    color = lv_color_hex(0xFF0000); // Red
                    break;
                default:
                    color = lv_color_hex(0x666666); // Default gray
                    break;
            }
            lv_obj_set_style_bg_color(circles[i], color, LV_PART_MAIN);
        }
    }
}

static void reset_match_data(void)
{
    match_me_wins = 0;
    match_draws = 0;
    match_opp_wins = 0;
    for (int i = 0; i < 3; i++) {
        match_rounds[i] = false;
        match_round_results[i] = 0;
    }
    update_match_display();
}

static void match_button_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int *data = (int*)lv_event_get_user_data(e);
    
    // Handle long press to close popup
    if (code == LV_EVENT_LONG_PRESSED && data == NULL) {
        ESP_LOGI(TAG, "Match popup long pressed - closing");
        haptic_feedback_short();
        hide_match_popup();
        return;
    }
    
    // Handle button clicks
    if (code == LV_EVENT_CLICKED && data != NULL) {
        int button_id = *data;
        
        // Handle reset button (special ID -1)
        if (button_id == -1) {
            ESP_LOGI(TAG, "Match reset button pressed");
            reset_match_data();
            haptic_feedback_short();
            hide_match_popup();
            return;
        }
        
        int round = button_id / 10;  // Extract round (0, 1, 2)
        int result = button_id % 10; // Extract result (1=me, 2=draw, 3=opp)
        
        ESP_LOGI(TAG, "Match button pressed - Round: %d, Result: %d", round, result);
        
        // Check if this round already has a result - if so, subtract the old result first
        if (match_rounds[round]) {
            int old_result = match_round_results[round];
            switch(old_result) {
                case 1: // Previous Me win
                    if (match_me_wins > 0) match_me_wins--;
                    break;
                case 2: // Previous Draw
                    if (match_draws > 0) match_draws--;
                    break;
                case 3: // Previous Opponent win
                    if (match_opp_wins > 0) match_opp_wins--;
                    break;
            }
        }
        
        // Update round result
        match_round_results[round] = result;
        match_rounds[round] = true;
        
        // Update counters based on the new result
        switch(result) {
            case 1: // Me wins
                match_me_wins++;
                break;
            case 2: // Draw
                match_draws++;
                break;
            case 3: // Opponent wins
                match_opp_wins++;
                break;
        }
        
        haptic_feedback_short();
        
        // Update display
        update_match_display();
        
        // Hide popup after selection
        hide_match_popup();
    }
}



static void match_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    // Check for swipe gesture first
    if (check_swipe_gesture(code)) {
        return; // Swipe was detected, don't process other events
    }
    
    // Long press functionality removed - use main menu instead
}

static void counter_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    // Check for swipe gesture first
    if (check_swipe_gesture(code)) {
        return; // Swipe was detected, don't process other events
    }
    
    // Handle long press (existing functionality)
    if (code == LV_EVENT_LONG_PRESSED) {
        counter_long_press_cb(e);
    }
}

static void show_match_popup(void)
{
    if (match_popup_open) return; // Prevent multiple popups
    
    match_popup_open = true;
    
    // Create circular popup container
    match_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(match_popup, 320, 320);
    lv_obj_center(match_popup);
    lv_obj_set_style_bg_color(match_popup, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_border_color(match_popup, lv_color_hex(0x0088FF), LV_PART_MAIN);
    lv_obj_set_style_border_width(match_popup, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(match_popup, 160, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(match_popup, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(match_popup, lv_color_black(), LV_PART_MAIN);
    
    // Create header labels
    lv_obj_t *header = lv_label_create(match_popup);
    lv_label_set_text(header, "Record Game Result");
    lv_obj_set_style_text_color(header, lv_color_hex(0x0088FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(header, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create grid for round selection (3x3 grid)
    static int button_data[9]; // Static to persist beyond function scope
    
    // Round headers
    const char* round_names[] = {"Game 1", "Game 2", "Game 3"};
    const char* result_names[] = {"Me", "Draw", "Opp"};
    
    for (int round = 0; round < 3; round++) {
        // Round label
        lv_obj_t *round_label = lv_label_create(match_popup);
        lv_label_set_text(round_label, round_names[round]);
        lv_obj_set_style_text_color(round_label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_font(round_label, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_align(round_label, LV_ALIGN_TOP_LEFT, 20, 60 + round * 70);
        
        for (int result = 0; result < 3; result++) {
            lv_obj_t *btn = lv_btn_create(match_popup);
            lv_obj_set_size(btn, 50, 30);
            lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 80 + result * 60, 50 + round * 70);
            
            // Set button color based on result type
            lv_color_t btn_color;
            switch(result) {
                case 0: btn_color = lv_color_hex(0x00AA00); break; // Green for "Me"
                case 1: btn_color = lv_color_hex(0xFFAA00); break; // Orange for "Draw"  
                case 2: btn_color = lv_color_hex(0xAA0000); break; // Red for "Opp"
            }
            lv_obj_set_style_bg_color(btn, btn_color, LV_PART_MAIN);
            
            // Mark button if already selected
            if (match_rounds[round] && match_round_results[round] == (result + 1)) {
                lv_obj_set_style_border_color(btn, lv_color_white(), LV_PART_MAIN);
                lv_obj_set_style_border_width(btn, 3, LV_PART_MAIN);
            }
            
            lv_obj_t *btn_label = lv_label_create(btn);
            lv_label_set_text(btn_label, result_names[result]);
            lv_obj_set_style_text_color(btn_label, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_12, LV_PART_MAIN);
            lv_obj_center(btn_label);
            
            // Store button data (round * 10 + result + 1)
            button_data[round * 3 + result] = round * 10 + (result + 1);
            lv_obj_add_event_cb(btn, match_button_cb, LV_EVENT_CLICKED, &button_data[round * 3 + result]);
        }
    }
    
    // Reset button
    lv_obj_t *reset_btn = lv_btn_create(match_popup);
    lv_obj_set_size(reset_btn, 100, 30);
    lv_obj_align(reset_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0x666666), LV_PART_MAIN);
    
    lv_obj_t *reset_label = lv_label_create(reset_btn);
    lv_label_set_text(reset_label, "Reset Match");
    lv_obj_set_style_text_color(reset_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(reset_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(reset_label);
    
    // Reset button callback - use special ID -1
    static int reset_data = -1;
    lv_obj_add_event_cb(reset_btn, match_button_cb, LV_EVENT_CLICKED, &reset_data);
    
    // Make popup clickable to close on long press
    lv_obj_add_flag(match_popup, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(match_popup, match_button_cb, LV_EVENT_LONG_PRESSED, NULL);
}

static void hide_match_popup(void)
{
    if (match_popup != NULL) {
        lv_obj_del(match_popup);
        match_popup = NULL;
    }
    match_popup_open = false;
}

// Main menu functions
static void update_main_menu_selection(void)
{
    for (int i = 0; i < 5; i++) {
        if (menu_items[i] != NULL) {
            if (i == main_menu_selected) {
                // Highlight selected item
                lv_obj_set_style_border_color(menu_items[i], lv_color_white(), LV_PART_MAIN);
                lv_obj_set_style_border_width(menu_items[i], 4, LV_PART_MAIN);
                lv_obj_set_style_bg_color(menu_items[i], menu_colors[i], LV_PART_MAIN);
            } else {
                // Normal appearance
                lv_obj_set_style_border_color(menu_items[i], menu_colors[i], LV_PART_MAIN);
                lv_obj_set_style_border_width(menu_items[i], 2, LV_PART_MAIN);
                lv_obj_set_style_bg_color(menu_items[i], lv_color_hex(0x333333), LV_PART_MAIN);
            }
        }
    }
}

static void main_menu_select_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Main menu item selected: %s", menu_labels[main_menu_selected]);
        
        // Add haptic feedback
        haptic_feedback_short();
        
        // TODO: Implement menu item actions
        switch(main_menu_selected) {
            case 0: // Timer
                ESP_LOGI(TAG, "Timer selected");
                hide_main_menu();
                show_timer_setting_popup();
                return; // Return early since menu is already hidden
            case 1: // Match
                ESP_LOGI(TAG, "Match report selected");
                hide_main_menu();
                show_match_popup();
                return; // Return early since menu is already hidden
            case 2: // Brightness
                ESP_LOGI(TAG, "Brightness selected");
                hide_main_menu();
                show_brightness_menu();
                return; // Return early since menu is already hidden
            case 3: // Colors
                ESP_LOGI(TAG, "Colors selected");
                hide_main_menu();
                show_color_menu();
                return; // Return early since menu is already hidden
            case 4: // Base
                ESP_LOGI(TAG, "Base selected");
                break;
        }
        
        // Close menu after selection
        hide_main_menu();
    } else if (code == LV_EVENT_LONG_PRESSED) {
        ESP_LOGI(TAG, "Main menu long pressed - closing");
        haptic_feedback_short();
        hide_main_menu();
    }
}

static void show_main_menu(void)
{
    if (main_menu_open) return; // Prevent multiple menus
    
    main_menu_open = true;
    
    // Create main menu container
    main_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_menu, 360, 360);
    lv_obj_center(main_menu);
    lv_obj_set_style_bg_color(main_menu, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(main_menu, 220, LV_PART_MAIN); // Semi-transparent
    lv_obj_set_style_border_opa(main_menu, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(main_menu, 20, LV_PART_MAIN); // Rounded corners instead of circle
    lv_obj_clear_flag(main_menu, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create center button (selection wheel indicator)
    menu_center_button = lv_btn_create(main_menu);
    lv_obj_set_size(menu_center_button, 100, 100);
    lv_obj_center(menu_center_button);
    lv_obj_set_style_bg_color(menu_center_button, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_radius(menu_center_button, 50, LV_PART_MAIN); // Circular
    lv_obj_add_event_cb(menu_center_button, main_menu_select_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *center_label = lv_label_create(menu_center_button);
    lv_label_set_text(center_label, "SELECT");
    lv_obj_set_style_text_color(center_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(center_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_center(center_label);
    
    // Create menu items in a 6-position layout with battery at top center
    // Position 0 (TOP): Battery indicator (non-selectable)
    // Position 1 (TOP-RIGHT): Timer  
    // Position 2 (BOTTOM-RIGHT): Match
    // Position 3 (BOTTOM): Brightness
    // Position 4 (BOTTOM-LEFT): Colors
    // Position 5 (TOP-LEFT): Base
    
    // First create the battery indicator at the top center
    create_battery_indicator(main_menu);
    
    for (int i = 0; i < 5; i++) {
        menu_items[i] = lv_btn_create(main_menu);
        lv_obj_set_size(menu_items[i], 80, 40);
        lv_obj_set_style_radius(menu_items[i], 8, LV_PART_MAIN);
        
        // Position buttons in a 6-position wheel (evenly distributed)
        // Each position is 60 degrees apart (360/6 = 60)
        switch(i) {
            case 0: // Timer - TOP-RIGHT (60 degrees)
                lv_obj_align(menu_items[i], LV_ALIGN_TOP_RIGHT, -20, 55);
                break;
            case 1: // Match - BOTTOM-RIGHT (120 degrees)  
                lv_obj_align(menu_items[i], LV_ALIGN_RIGHT_MID, -20, 35);
                break;
            case 2: // Brightness - BOTTOM (180 degrees)
                lv_obj_align(menu_items[i], LV_ALIGN_BOTTOM_MID, 0, -20);
                break;
            case 3: // Colors - BOTTOM-LEFT (240 degrees)
                lv_obj_align(menu_items[i], LV_ALIGN_LEFT_MID, 20, 35);
                break;
            case 4: // Base - TOP-LEFT (300 degrees)
                lv_obj_align(menu_items[i], LV_ALIGN_TOP_LEFT, 20, 55);
                break;
        }
        
        lv_obj_t *label = lv_label_create(menu_items[i]);
        lv_label_set_text(label, menu_labels[i]);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_center(label);
    }
    
    // Update initial selection
    update_main_menu_selection();
    
    ESP_LOGI(TAG, "Main menu opened");
}

static void hide_main_menu(void)
{
    if (main_menu != NULL) {
        lv_obj_del(main_menu);
        main_menu = NULL;
        
        // Clear menu item references
        for (int i = 0; i < 5; i++) {
            menu_items[i] = NULL;
        }
        menu_center_button = NULL;
        
        // Clear battery indicator references
        battery_icon = NULL;
        battery_percentage_label = NULL;
        battery_time_label = NULL;
    }
    main_menu_open = false;
    ESP_LOGI(TAG, "Main menu closed");
}

static void swipe_up_detected(void)
{
    ESP_LOGI(TAG, "Swipe up detected - opening main menu");
    haptic_feedback_short();
    show_main_menu();
}

static void create_counter_ui(void)
{
    // Create a screen
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);  // Disable scrolling on main screen
    
    // Create timer label at the top
    timer_label = lv_label_create(scr);
    // Initial display will be set by update_timer_display()
    lv_obj_set_style_text_color(timer_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(timer_label, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_set_style_text_align(timer_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(timer_label, LV_ALIGN_TOP_MID, 0, 30);
    
    // Create alarm message label (initially hidden)
    alarm_message_label = lv_label_create(scr);
    lv_label_set_text(alarm_message_label, "Round Ended");
    lv_obj_set_style_text_color(alarm_message_label, lv_color_hex(0xFF0000), LV_PART_MAIN); // Red text
    lv_obj_set_style_text_font(alarm_message_label, &lv_font_montserrat_16, LV_PART_MAIN); // Smaller font
    lv_obj_set_style_text_align(alarm_message_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(alarm_message_label, LV_ALIGN_TOP_MID, 0, 75); // Closer to timer label
    lv_obj_add_flag(alarm_message_label, LV_OBJ_FLAG_HIDDEN); // Start hidden
    
    // Make timer label clickable (for future features if needed)
    lv_obj_add_flag(timer_label, LV_OBJ_FLAG_CLICKABLE);
    
    // Create invisible touch area for top 1/3 of screen (120 pixels high on 360px screen)
    timer_touch_area = lv_obj_create(scr);
    lv_obj_set_size(timer_touch_area, 360, 120);  // Full width, top 1/3 height
    lv_obj_align(timer_touch_area, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(timer_touch_area, 0, LV_PART_MAIN);  // Make invisible
    lv_obj_set_style_border_opa(timer_touch_area, 0, LV_PART_MAIN);  // No border
    lv_obj_add_flag(timer_touch_area, LV_OBJ_FLAG_CLICKABLE);
    // Single callback handles both click and long press
    lv_obj_add_event_cb(timer_touch_area, timer_touch_cb, LV_EVENT_ALL, NULL);
    
    // Create a large label for the counter in the center
    counter_label = lv_label_create(scr);
    lv_label_set_text(counter_label, "0");
    lv_obj_set_style_text_color(counter_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(counter_label, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_align(counter_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_center(counter_label);
    
    // Create invisible touch area for middle 1/3 of screen (counter area)
    lv_obj_t *counter_touch_area = lv_obj_create(scr);
    lv_obj_set_size(counter_touch_area, 360, 120);  // Full width, middle 1/3 height
    lv_obj_set_pos(counter_touch_area, 0, 120);  // Position at middle third (120-240)
    lv_obj_set_style_bg_opa(counter_touch_area, 0, LV_PART_MAIN);  // Make invisible
    lv_obj_set_style_border_opa(counter_touch_area, 0, LV_PART_MAIN);  // No border
    lv_obj_add_flag(counter_touch_area, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(counter_touch_area, counter_touch_cb, LV_EVENT_ALL, NULL);
    
    // Make the label clickable for long press reset (keep existing functionality)
    lv_obj_add_flag(counter_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(counter_label, counter_long_press_cb, LV_EVENT_LONG_PRESSED, NULL);
    
    // === MATCH COUNTER SECTION (Bottom 1/3) ===
    // Match counter labels positioned directly on screen for stability
    
    // Create three game result circles (from left to right: Game 1, Game 2, Game 3)
    match_circle_1 = lv_obj_create(scr);  // Game 1 circle
    lv_obj_set_size(match_circle_1, 30, 30);
    lv_obj_set_style_radius(match_circle_1, 15, LV_PART_MAIN); // Make circular
    lv_obj_set_style_bg_color(match_circle_1, lv_color_hex(0x666666), LV_PART_MAIN); // Default gray
    lv_obj_set_style_border_width(match_circle_1, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(match_circle_1, lv_color_hex(0x999999), LV_PART_MAIN);
    lv_obj_set_pos(match_circle_1, 120, 290);  // Left position
    lv_obj_clear_flag(match_circle_1, LV_OBJ_FLAG_SCROLLABLE);
    
    match_circle_2 = lv_obj_create(scr);  // Game 2 circle
    lv_obj_set_size(match_circle_2, 30, 30);
    lv_obj_set_style_radius(match_circle_2, 15, LV_PART_MAIN); // Make circular
    lv_obj_set_style_bg_color(match_circle_2, lv_color_hex(0x666666), LV_PART_MAIN); // Default gray
    lv_obj_set_style_border_width(match_circle_2, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(match_circle_2, lv_color_hex(0x999999), LV_PART_MAIN);
    lv_obj_set_pos(match_circle_2, 165, 290);  // Center position
    lv_obj_clear_flag(match_circle_2, LV_OBJ_FLAG_SCROLLABLE);
    
    match_circle_3 = lv_obj_create(scr);  // Game 3 circle
    lv_obj_set_size(match_circle_3, 30, 30);
    lv_obj_set_style_radius(match_circle_3, 15, LV_PART_MAIN); // Make circular
    lv_obj_set_style_bg_color(match_circle_3, lv_color_hex(0x666666), LV_PART_MAIN); // Default gray
    lv_obj_set_style_border_width(match_circle_3, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(match_circle_3, lv_color_hex(0x999999), LV_PART_MAIN);
    lv_obj_set_pos(match_circle_3, 210, 290);  // Right position
    lv_obj_clear_flag(match_circle_3, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create invisible touch area for match counter - use absolute positioning
    match_touch_area = lv_obj_create(scr);  // Create on screen, not container
    lv_obj_set_size(match_touch_area, 360, 120);  // Full bottom area
    lv_obj_set_pos(match_touch_area, 0, 240);  // Position at bottom third
    lv_obj_set_style_bg_opa(match_touch_area, 0, LV_PART_MAIN);  // Make invisible
    lv_obj_set_style_border_opa(match_touch_area, 0, LV_PART_MAIN);  // No border
    lv_obj_set_style_outline_opa(match_touch_area, 0, LV_PART_MAIN);  // No outline
    lv_obj_set_style_shadow_opa(match_touch_area, 0, LV_PART_MAIN);  // No shadow
    lv_obj_clear_flag(match_touch_area, LV_OBJ_FLAG_SCROLLABLE);  // Disable scrolling
    lv_obj_add_flag(match_touch_area, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(match_touch_area, match_touch_cb, LV_EVENT_ALL, NULL);
}

static void update_counter_display(void)
{
    if (counter_label != NULL) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%ld", counter_value);
        lv_label_set_text(counter_label, buf);
    }
}

static void encoder_task(void *arg)
{
    ESP_LOGI(TAG, "Starting encoder task");
    
    while (1) {
        EventBits_t events = xEventGroupWaitBits(knob_even_, 0x07, pdTRUE, pdFALSE, pdMS_TO_TICKS(100));
        
        // Handle encoder button press (0x04)
        if (events & 0x04) {
            if (main_menu_open) {
                // Main menu is open - center button selects current option
                ESP_LOGI(TAG, "Main menu selection: %ld", main_menu_selected);
                
                // Haptic feedback for selection
                haptic_feedback_short();
                
                // Handle menu selection
                if (example_lvgl_lock(-1)) {
                    hide_main_menu();
                    example_lvgl_unlock();
                }
                
                // TODO: Add actual menu action handling here
                // switch(main_menu_selected) {
                //     case 0: // Timer
                //     case 1: // Brightness
                //     case 2: // Colors
                //     case 3: // Base Settings
                // }
                
            } else if (brightness_menu_open) {
                // Close brightness menu on button press
                ESP_LOGI(TAG, "Brightness menu closed via button press");
                haptic_feedback_short();
                
                if (example_lvgl_lock(-1)) {
                    hide_brightness_menu();
                    example_lvgl_unlock();
                }
            } else if (color_menu_open) {
                // Select color on button press
                ESP_LOGI(TAG, "Color selected via button press: %s", color_names[selected_color_index]);
                haptic_feedback_short();
                
                if (example_lvgl_lock(-1)) {
                    hide_color_menu();
                    example_lvgl_unlock();
                }
            } else if (!timer_setting_mode) {
                ESP_LOGI(TAG, "Encoder button pressed - reset counter");
                counter_value = 0;
                if (example_lvgl_lock(-1)) {
                    update_counter_display();
                    example_lvgl_unlock();
                }
            }
            // Note: In timer setting mode, use long press on timer label to close popup
        }
        
        if (events & 0x01) { // Left rotation (decrement)
            if (main_menu_open) {
                // Navigate main menu left (counter-clockwise)
                main_menu_selected = (main_menu_selected == 0) ? 4 : main_menu_selected - 1;
                ESP_LOGI(TAG, "Main menu navigation: %ld", main_menu_selected);
                
                // Haptic feedback for menu navigation
                haptic_feedback_short();
                
                if (example_lvgl_lock(-1)) {
                    update_main_menu_selection();
                    example_lvgl_unlock();
                }
            } else if (brightness_menu_open) {
                // Decrease brightness
                if (brightness_level > 15) { // Minimum brightness (not completely dark)
                    brightness_level -= 10;
                    ESP_LOGI(TAG, "Brightness decreased to: %d", brightness_level);
                    
                    haptic_feedback_short();
                    
                    if (example_lvgl_lock(-1)) {
                        update_brightness_display();
                        example_lvgl_unlock();
                    }
                }
            } else if (color_menu_open) {
                // Navigate through colors (left = previous)
                selected_color_index = (selected_color_index == 0) ? 5 : selected_color_index - 1;
                ESP_LOGI(TAG, "Color selection: %s", color_names[selected_color_index]);
                
                haptic_feedback_short();
                
                if (example_lvgl_lock(-1)) {
                    update_color_selection();
                    example_lvgl_unlock();
                }
            } else if (timer_setting_mode) {
                // Decrease timer minutes (minimum 1 minute)
                if (timer_setting_minutes > 1) {
                    timer_setting_minutes--;
                    ESP_LOGI(TAG, "Timer setting decremented to: %u minutes", timer_setting_minutes);
                    
                    // Haptic feedback for timer decrement
                    haptic_feedback_short();
                    
                    if (example_lvgl_lock(-1)) {
                        update_timer_setting_display();
                        example_lvgl_unlock();
                    }
                }
            } else {
                // Decrease counter (minimum 0)
                if (counter_value > 0) {
                    counter_value--;
                    ESP_LOGI(TAG, "Counter decremented to: %ld", counter_value);
                    
                    // Haptic feedback for counter decrement
                    haptic_feedback_short();
                    
                    // Update display in LVGL thread-safe way
                    if (example_lvgl_lock(-1)) {
                        update_counter_display();
                        example_lvgl_unlock();
                    }
                }
            }
        }
        
        if (events & 0x02) { // Right rotation (increment)
            if (main_menu_open) {
                // Navigate main menu right (clockwise)
                main_menu_selected = (main_menu_selected == 4) ? 0 : main_menu_selected + 1;
                ESP_LOGI(TAG, "Main menu navigation: %ld", main_menu_selected);
                
                // Haptic feedback for menu navigation
                haptic_feedback_short();
                
                if (example_lvgl_lock(-1)) {
                    update_main_menu_selection();
                    example_lvgl_unlock();
                }
            } else if (brightness_menu_open) {
                // Increase brightness
                if (brightness_level < 255) { // Maximum brightness
                    brightness_level += 10;
                    ESP_LOGI(TAG, "Brightness increased to: %d", brightness_level);
                    
                    haptic_feedback_short();
                    
                    if (example_lvgl_lock(-1)) {
                        update_brightness_display();
                        example_lvgl_unlock();
                    }
                }
            } else if (color_menu_open) {
                // Navigate through colors (right = next)
                selected_color_index = (selected_color_index == 5) ? 0 : selected_color_index + 1;
                ESP_LOGI(TAG, "Color selection: %s", color_names[selected_color_index]);
                
                haptic_feedback_short();
                
                if (example_lvgl_lock(-1)) {
                    update_color_selection();
                    example_lvgl_unlock();
                }
            } else if (timer_setting_mode) {
                // Increase timer minutes (maximum 60 minutes)
                if (timer_setting_minutes < 60) {
                    timer_setting_minutes++;
                    ESP_LOGI(TAG, "Timer setting incremented to: %u minutes", timer_setting_minutes);
                    
                    // Haptic feedback for timer increment
                    haptic_feedback_short();
                    
                    if (example_lvgl_lock(-1)) {
                        update_timer_setting_display();
                        example_lvgl_unlock();
                    }
                }
            } else {
                counter_value++;
                ESP_LOGI(TAG, "Counter incremented to: %ld", counter_value);
                
                // Haptic feedback for counter increment
                haptic_feedback_short();
                
                // Update display in LVGL thread-safe way
                if (example_lvgl_lock(-1)) {
                    update_counter_display();
                    example_lvgl_unlock();
                }
            }
        }
        
        // Update timer display regularly (every 100ms) - but not during timer setting
        if (!timer_setting_mode && example_lvgl_lock(-1)) {
            update_timer_display();
            
            // Update battery display if main menu is open
            if (main_menu_open) {
                update_battery_display();
            }
            
            example_lvgl_unlock();
        }
    }
}

static void example_lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    while (1) {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        if (example_lvgl_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            // Release the mutex
            example_lvgl_unlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}
#ifdef Backlight_Testing
void example_backlight_test_task(void *arg)
{
    for(;;)
    {
        setUpduty(LCD_PWM_MODE_255);
        vTaskDelay(pdMS_TO_TICKS(1000));
        setUpduty(LCD_PWM_MODE_200);
        vTaskDelay(pdMS_TO_TICKS(1000));
        setUpduty(LCD_PWM_MODE_150);
        vTaskDelay(pdMS_TO_TICKS(1000));
        setUpduty(LCD_PWM_MODE_100);
        vTaskDelay(pdMS_TO_TICKS(1000));
        setUpduty(LCD_PWM_MODE_50);
        vTaskDelay(pdMS_TO_TICKS(1000));
        setUpduty(LCD_PWM_MODE_0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#endif
void app_main(void)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions

    lcd_bl_pwm_bsp_init(LCD_PWM_MODE_255);

    ESP_LOGI(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = 
    {
        .data0_io_num = EXAMPLE_PIN_NUM_LCD_DATA0,
        .data1_io_num = EXAMPLE_PIN_NUM_LCD_DATA1,
        .sclk_io_num = EXAMPLE_PIN_NUM_LCD_PCLK,
        .data2_io_num = EXAMPLE_PIN_NUM_LCD_DATA2,
        .data3_io_num = EXAMPLE_PIN_NUM_LCD_DATA3,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = SH8601_PANEL_IO_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS,
                                                                                example_notify_lvgl_flush_ready,
                                                                                &disp_drv);
    sh8601_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags = {
            .use_qspi_interface = 1,
        },
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
        .vendor_config = &vendor_config,
    };
    ESP_LOGI(TAG, "Install SH8601 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    i2c_master_Init(); //I2C_Init
    
    // Initialize DRV2605 haptic feedback
    ESP_LOGI(TAG, "Initialize DRV2605 haptic motor");
    i2c_drv2605_setup();
#if EXAMPLE_USE_TOUCH
    lcd_touch_init();
#endif

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    //alloc draw buffers used by LVGL
    //it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    //initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LVGL_BUF_HEIGHT);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.rounder_cb = example_lvgl_rounder_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    //Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

#if EXAMPLE_USE_TOUCH
    static lv_indev_drv_t indev_drv;           // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = example_lvgl_touch_cb;
    // Set long press time for timer setting (600ms for reliable detection)
    indev_drv.long_press_time = 600;
    lv_indev_drv_register(&indev_drv);
#endif

    lvgl_mux = xSemaphoreCreateMutex();
    assert(lvgl_mux);
    xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);
    
    // Initialize rotary encoder
    ESP_LOGI(TAG, "Initialize rotary encoder");
    user_encoder_init();
    
    // Create encoder handling task
    xTaskCreate(encoder_task, "encoder", 4096, NULL, 3, NULL);
    
#ifdef Backlight_Testing
    xTaskCreate(example_backlight_test_task, "backlight", 3 * 1024, NULL, 2, NULL);
#endif
    ESP_LOGI(TAG, "=== BUILDING CUSTOM COUNTER APPLICATION ===");
    ESP_LOGI(TAG, "Create counter UI");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (example_lvgl_lock(-1))
    {
        create_counter_ui();
        // Release the mutex
        example_lvgl_unlock();
    }
    
    // Initialize timer variables
    timer_seconds = 3000; // 50 minutes
    timer_running = false;
    timer_start_time = 0;
    timer_elapsed_total = 0;
    timer_handle = NULL;
    
    // Update initial timer display
    if (example_lvgl_lock(-1))
    {
        update_timer_display();
        example_lvgl_unlock();
    }
    
    // Initialize match counter variables
    match_me_wins = 0;
    match_draws = 0;
    match_opp_wins = 0;
    match_popup_open = false;
    for (int i = 0; i < 3; i++) {
        match_rounds[i] = false;
        match_round_results[i] = 0;
    }
    
    // Update initial match display
    if (example_lvgl_lock(-1))
    {
        update_match_display();
        example_lvgl_unlock();
    }
    
    ESP_LOGI(TAG, "Timer initialized");
    ESP_LOGI(TAG, "Match counter initialized");
}
