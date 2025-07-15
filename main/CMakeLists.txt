# Main component CMakeLists.txt

idf_component_register(
    SRCS "main.c" "lcd_driver.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_lcd spi_flash esp_timer freertos
)
