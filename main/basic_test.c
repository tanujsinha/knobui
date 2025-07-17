#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "BASIC_TEST";

// Test all possible backlight pins
int backlight_pins[] = {2, 47, 48, 35, 36, 37, 38, 39, 40, 41, 42, 45, 46, -1};
int reset_pins[] = {8, 21, 1, 3, 4, 5, 6, 7, -1};

void basic_lcd_test(void)
{
    ESP_LOGI(TAG, "Starting basic LCD pin test");
    
    // Test backlight pins
    for (int i = 0; backlight_pins[i] != -1; i++) {
        int pin = backlight_pins[i];
        ESP_LOGI(TAG, "Testing backlight pin %d", pin);
        
        gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1ULL << pin),
            .pull_down_en = 0,
            .pull_up_en = 0,
        };
        
        esp_err_t ret = gpio_config(&io_conf);
        if (ret == ESP_OK) {
            // Blink the pin
            for (int j = 0; j < 5; j++) {
                gpio_set_level(pin, 1);
                vTaskDelay(pdMS_TO_TICKS(500));
                gpio_set_level(pin, 0);
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            ESP_LOGI(TAG, "Tested pin %d", pin);
        } else {
            ESP_LOGI(TAG, "Pin %d config failed", pin);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    ESP_LOGI(TAG, "Basic test complete");
}
