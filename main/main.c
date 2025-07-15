#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lcd_driver.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    // Initialize NVS (required for some ESP32 features)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP32-S3 Knob Touch LCD Test Starting...");
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());

    // Initialize LCD
    ESP_LOGI(TAG, "Initializing LCD display...");
    ret = lcd_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "LCD initialized successfully");

    // Clear screen with black background
    ESP_LOGI(TAG, "Clearing screen...");
    lcd_fill_screen(COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(500));

    // Set backlight to full brightness
    lcd_set_brightness(80);

    // Draw test content
    ESP_LOGI(TAG, "Drawing test content...");
    
    // Draw title
    lcd_draw_text(100, 50, "HELLO WORLD!", COLOR_WHITE, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Draw device info
    lcd_draw_text(80, 100, "ESP LCD TEST", COLOR_CYAN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Draw status
    lcd_draw_text(120, 150, "STATUS OK", COLOR_GREEN, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Draw some colored rectangles as visual test
    ESP_LOGI(TAG, "Drawing colored rectangles...");
    lcd_fill_rect(50, 200, 60, 30, COLOR_RED);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    lcd_fill_rect(130, 200, 60, 30, COLOR_GREEN);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    lcd_fill_rect(210, 200, 60, 30, COLOR_BLUE);
    vTaskDelay(pdMS_TO_TICKS(500));

    // Draw border around screen
    ESP_LOGI(TAG, "Drawing screen border...");
    lcd_fill_rect(0, 0, LCD_WIDTH, 5, COLOR_WHITE);        // Top
    lcd_fill_rect(0, LCD_HEIGHT-5, LCD_WIDTH, 5, COLOR_WHITE);  // Bottom
    lcd_fill_rect(0, 0, 5, LCD_HEIGHT, COLOR_WHITE);       // Left
    lcd_fill_rect(LCD_WIDTH-5, 0, 5, LCD_HEIGHT, COLOR_WHITE);  // Right

    ESP_LOGI(TAG, "Test display complete!");
    ESP_LOGI(TAG, "The LCD should now show:");
    ESP_LOGI(TAG, "- 'HELLO WORLD!' at the top");
    ESP_LOGI(TAG, "- 'ESP LCD TEST' in the middle");
    ESP_LOGI(TAG, "- 'STATUS OK' below that");
    ESP_LOGI(TAG, "- Three colored rectangles (red, green, blue)");
    ESP_LOGI(TAG, "- White border around the screen");

    // Main loop - blink status indicator
    bool status_visible = true;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Toggle status text visibility as a "heartbeat" indicator
        if (status_visible) {
            lcd_fill_rect(120, 150, 130, 16, COLOR_BLACK); // Clear status area
            ESP_LOGI(TAG, "Heartbeat: Status hidden");
        } else {
            lcd_draw_text(120, 150, "STATUS OK", COLOR_GREEN, COLOR_BLACK);
            ESP_LOGI(TAG, "Heartbeat: Status visible");
        }
        status_visible = !status_visible;
        
        // Print memory info periodically
        ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    }
}
