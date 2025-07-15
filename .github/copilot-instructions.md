<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# ESP32-S3 Knob Touch LCD Development

This is an ESP-IDF project for the Waveshare ESP32-S3-Knob-Touch-LCD-1.8 development board.

## Device Specifications
- **MCU**: ESP32-S3 (dual-core, Wi-Fi, Bluetooth)
- **Display**: 1.8" LCD with 360x360 resolution
- **Touch**: Capacitive touch controller (CST816)
- **Audio**: PCM5100A audio codec with 3.5mm jack
- **Input**: Rotary encoder with push button
- **Haptic**: DRV2605 vibration motor driver
- **Storage**: MicroSD card slot
- **Power**: USB-C, battery connector (PH1.25)

## Development Guidelines
- Use ESP-IDF framework (version 5.x recommended)
- Follow ESP32-S3 hardware specifications
- Use LVGL library for GUI development
- Configure proper GPIO pins for peripherals
- Include proper error handling for hardware initialization
- Use FreeRTOS tasks for concurrent operations

## Hardware Pin Configuration
- **LCD SPI**: Uses FSPI interface
- **Touch I2C**: Uses I2C interface
- **Audio I2S**: Uses I2S interface
- **SD Card**: Uses SDMMC interface
- **Encoder**: Uses GPIO with interrupt handling
- **Vibration**: Uses I2C interface (DRV2605)

## Code Style
- Follow ESP-IDF coding standards
- Use descriptive variable and function names
- Include proper documentation for hardware interfaces
- Implement proper initialization sequences for peripherals
