#include "lcd_driver.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "LCD_DRIVER";

static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;
static spi_device_handle_t spi_device = NULL;

// Simple 8x8 font bitmap (basic characters)
static const uint8_t font_8x8[][8] = {
    // 0: Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 1: ! (33)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},
    // 2: 0 (48)
    {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
    // 3: 1 (49)
    {0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E, 0x00},
    // 4: 2 (50)
    {0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00},
    // 5: 3 (51)
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    // 6: : (58)
    {0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00},
    // 7: A (65)
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
    // 8: B (66)
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    // 9: C (67)
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    // 10: D (68)
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    // 11: E (69)
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00},
    // 12: F (70)
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00},
    // 13: G (71)
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00},
    // 14: H (72)
    {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    // 15: I (73)
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    // 16: J (74)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x6C, 0x6C, 0x38, 0x00},
    // 17: K (75)
    {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00},
    // 18: L (76)
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    // 19: M (77)
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},
    // 20: N (78)
    {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    // 21: O (79)
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    // 22: P (80)
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    // 23: Q (81)
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x0E, 0x00},
    // 24: R (82)
    {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00},
    // 25: S (83)
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
    // 26: T (84)
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    // 27: U (85)
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    // 28: V (86)
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    // 29: W (87)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
    // 30: X (88)
    {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},
    // 31: Y (89)
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
    // 32: Z (90)
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},
};

esp_err_t lcd_init(void)
{
    ESP_LOGI(TAG, "Initializing LCD");
    
    // Initialize GPIO pins (Reset and Backlight only, no DC pin for QSPI)
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LCD_PIN_NUM_RST) | (1ULL << LCD_PIN_NUM_BL),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Initialize SPI bus with standard SPI configuration (try this first)
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = LCD_PIN_NUM_MOSI,  // D0 (GPIO 15)
        .sclk_io_num = LCD_PIN_NUM_CLK,   // CLK (GPIO 13)
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Configure LCD panel IO (standard SPI mode with DC pin)
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_PIN_NUM_RST,  // Use RST as DC for now
        .cs_gpio_num = LCD_PIN_NUM_CS,
        .pclk_hz = 20 * 1000 * 1000,  // 20MHz (lower speed)
        .lcd_cmd_bits = 8,  // Standard command bits
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

    // Configure LCD panel for ST7789 controller
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,  // We'll handle reset manually
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    
    // Use ST7789 controller as fallback (widely compatible)
    ESP_LOGI(TAG, "Installing ST7789 LCD driver");
    esp_err_t ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ST7789 LCD panel initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Manual reset sequence
    ESP_LOGI(TAG, "Performing manual LCD reset");
    gpio_set_level(LCD_PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(LCD_PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Reset and initialize the panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    
    // Set orientation and gap
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 0));

    // Turn on display
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // Set backlight to maximum using GPIO 47 (simple GPIO control)
    ESP_LOGI(TAG, "Setting backlight to maximum (GPIO 47)");
    gpio_set_level(LCD_PIN_NUM_BL, 1);
    
    // Add a small delay to ensure everything is initialized
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "LCD initialization completed");
    return ESP_OK;
}

esp_err_t lcd_fill_screen(uint16_t color)
{
    if (panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // Allocate buffer for full screen
    size_t buffer_size = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t);
    uint16_t *buffer = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate screen buffer");
        return ESP_ERR_NO_MEM;
    }

    // Fill buffer with color
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        buffer[i] = color;
    }

    // Draw to screen
    esp_err_t ret = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, buffer);
    
    free(buffer);
    return ret;
}

esp_err_t lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    if (panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    // Clamp dimensions
    if (x + width > LCD_WIDTH) width = LCD_WIDTH - x;
    if (y + height > LCD_HEIGHT) height = LCD_HEIGHT - y;

    size_t buffer_size = width * height * sizeof(uint16_t);
    uint16_t *buffer = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate rect buffer");
        return ESP_ERR_NO_MEM;
    }

    // Fill buffer with color
    for (int i = 0; i < width * height; i++) {
        buffer[i] = color;
    }

    esp_err_t ret = esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + width, y + height, buffer);
    
    free(buffer);
    return ret;
}

esp_err_t lcd_draw_text(uint16_t x, uint16_t y, const char* text, uint16_t color, uint16_t bg_color)
{
    if (panel_handle == NULL || text == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    int len = strlen(text);
    uint16_t char_width = 8;
    uint16_t char_height = 8;
    
    for (int i = 0; i < len; i++) {
        char c = text[i];
        uint16_t char_x = x + (i * char_width);
        
        if (char_x + char_width > LCD_WIDTH) break;
        if (y + char_height > LCD_HEIGHT) break;

        // Get font data (safe mapping)
        const uint8_t *font_data;
        if (c == ' ') {
            font_data = font_8x8[0]; // Space
        } else if (c == '!') {
            font_data = font_8x8[1]; // Exclamation
        } else if (c >= '0' && c <= '3') {
            font_data = font_8x8[2 + (c - '0')]; // Digits 0-3
        } else if (c == ':') {
            font_data = font_8x8[6]; // Colon
        } else if (c >= 'A' && c <= 'Z') {
            font_data = font_8x8[7 + (c - 'A')]; // Letters A-Z
        } else {
            font_data = font_8x8[0]; // Default to space for unsupported chars
        }

        // Draw character
        uint16_t *char_buffer = heap_caps_malloc(char_width * char_height * sizeof(uint16_t), MALLOC_CAP_DMA);
        if (char_buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate char buffer");
            return ESP_ERR_NO_MEM;
        }

        for (int row = 0; row < char_height; row++) {
            uint8_t line = font_data[row];
            for (int col = 0; col < char_width; col++) {
                if (line & (0x80 >> col)) {
                    char_buffer[row * char_width + col] = color;
                } else {
                    char_buffer[row * char_width + col] = bg_color;
                }
            }
        }

        esp_lcd_panel_draw_bitmap(panel_handle, char_x, y, char_x + char_width, y + char_height, char_buffer);
        free(char_buffer);
    }

    return ESP_OK;
}

esp_err_t lcd_set_brightness(uint8_t brightness)
{
    if (brightness > 100) brightness = 100;
    
    uint32_t duty = (brightness * 255) / 100;
    return ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty) || 
           ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
