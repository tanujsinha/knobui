#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// LCD specifications for ESP32-S3-Knob-Touch-LCD-1.8
#define LCD_WIDTH       360
#define LCD_HEIGHT      360
#define LCD_BIT_PER_PIXEL 16

// GPIO pin definitions (adjust based on actual board connections)
#define LCD_PIN_NUM_MOSI    11
#define LCD_PIN_NUM_CLK     12
#define LCD_PIN_NUM_CS      10
#define LCD_PIN_NUM_DC      13
#define LCD_PIN_NUM_RST     14
#define LCD_PIN_NUM_BL      15

// Color definitions (RGB565 format)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F

/**
 * @brief Initialize the LCD display
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t lcd_init(void);

/**
 * @brief Fill the entire screen with a solid color
 * 
 * @param color RGB565 color value
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t lcd_fill_screen(uint16_t color);

/**
 * @brief Draw a filled rectangle
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param color RGB565 color value
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

/**
 * @brief Draw simple text on the screen
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param text Text string to display
 * @param color Text color (RGB565)
 * @param bg_color Background color (RGB565)
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t lcd_draw_text(uint16_t x, uint16_t y, const char* text, uint16_t color, uint16_t bg_color);

/**
 * @brief Set LCD backlight brightness
 * 
 * @param brightness Brightness level (0-100)
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t lcd_set_brightness(uint8_t brightness);

#ifdef __cplusplus
}
#endif
