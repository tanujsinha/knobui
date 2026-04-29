# ESP32-S3 Knob Touch LCD — Project Documentation

> **Board**: Waveshare ESP32-S3-Knob-Touch-LCD-1.8  
> **Framework**: ESP-IDF v5.x + LVGL 8.3.11  
> **UI Designer**: SquareLine Studio 1.5.3  
> **Project Name**: LVGL_Test (Smartwatch UI)

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Hardware Specifications](#2-hardware-specifications)
3. [Pin Configuration](#3-pin-configuration)
4. [Software Architecture](#4-software-architecture)
5. [Directory Structure](#5-directory-structure)
6. [Components Reference](#6-components-reference)
7. [UI System](#7-ui-system)
8. [FreeRTOS Task Model](#8-freertos-task-model)
9. [Build Configuration](#9-build-configuration)
10. [Build & Flash Workflow](#10-build--flash-workflow)
11. [Demo Examples](#11-demo-examples)
12. [Adding New Features](#12-adding-new-features)
13. [Troubleshooting](#13-troubleshooting)

---

## 1. Project Overview

This firmware runs a **smartwatch-style graphical interface** on a 360×360 round LCD display. The user interacts through a **rotary encoder** (scroll/click) and **capacitive touch**. Selections trigger **haptic feedback** via a DRV2605 vibration motor driver.

The UI is authored in **SquareLine Studio** and exported as LVGL 8.x C code. The application layer in `main.c` initialises all hardware peripherals, starts the LVGL rendering pipeline, and spawns FreeRTOS tasks to handle input, rendering, and haptics concurrently.

### Feature Summary

| Feature | Status | Details |
|---------|--------|---------|
| LCD Display | ✅ | SH8601 controller, QSPI, 360×360 |
| Touch Input | ✅ | CST816 capacitive, I2C |
| Rotary Encoder | ✅ | GPIO 7/8, EventGroup-based |
| Haptic Feedback | ✅ | DRV2605, ERM motor, library 5 |
| Backlight PWM | ✅ | GPIO 47, LEDC peripheral |
| Animated Watch UI | ✅ | Analog hands (sec/min/hour) |
| Color Wheel Screen | ✅ | LVGL colorwheel widget |
| Menu Navigation | ✅ | Infinite roller menu |
| PSRAM | ✅ | 8 MB Octal PSRAM at 80 MHz |
| SD Card | 🔲 | Hardware present, not in main app |
| Audio (PCM5100A) | 🔲 | Hardware present, demo only |
| Wi-Fi | 🔲 | Hardware present, demo only |

---

## 2. Hardware Specifications

| Component | Part | Notes |
|-----------|------|-------|
| MCU | ESP32-S3 | Dual-core Xtensa LX7, 240 MHz |
| Flash | 8 MB | QIO mode |
| PSRAM | 8 MB | Octal SPI, 80 MHz |
| Display | 1.8" round LCD | 360×360 px, SH8601 controller |
| Touch | CST816 | I2C, capacitive single-touch |
| Haptic | DRV2605 | I2C, ERM vibration motor |
| Audio | PCM5100A | I2S, 3.5mm stereo jack |
| Input | Rotary encoder | With push button |
| Storage | MicroSD | SDMMC interface |
| Power | USB-C + PH1.25 | Battery connector |

### Memory Layout

```
Flash (8 MB):
  0x0000 — Bootloader
  NVS    — 24 KB  (non-volatile storage)
  PHY    — 4 KB   (RF calibration)
  App    — 7 MB   (factory partition)

RAM:
  Internal SRAM  — ~512 KB
  PSRAM (Octal)  — 8 MB (heap, LVGL buffers)
```

---

## 3. Pin Configuration

All pins are defined in [`main/user_config.h`](main/user_config.h).

### LCD — QSPI (SPI2_HOST / FSPI)

| Signal | GPIO |
|--------|------|
| CS | 14 |
| CLK (PCLK) | 13 |
| DATA0 (MOSI) | 15 |
| DATA1 | 16 |
| DATA2 | 17 |
| DATA3 | 18 |
| RST | 21 |
| Backlight | 47 |

### I2C Bus (I2C_NUM_0)

| Signal | GPIO |
|--------|------|
| SCL | 12 |
| SDA | 11 |

### I2C Devices

| Device | Address |
|--------|---------|
| CST816 Touch | `0x15` |
| DRV2605 Haptic | `0x5A` |

### Touch Controller (CST816)

| Signal | GPIO |
|--------|------|
| RST | 10 |
| INT | 9 |

### Rotary Encoder

| Signal | GPIO |
|--------|------|
| ECA (Phase A) | 8 |
| ECB (Phase B) | 7 |

---

## 4. Software Architecture

### Startup Sequence (`app_main`)

```
app_main()
 │
 ├─ lcd_bl_pwm_bsp_init(LCD_PWM_MODE_255)   ← backlight on full brightness
 ├─ spi_bus_initialize(SPI2_HOST, ...)       ← QSPI bus for LCD
 ├─ esp_lcd_new_panel_io_spi(...)            ← SH8601 panel I/O handle
 ├─ esp_lcd_new_panel_sh8601(...)            ← LCD panel handle + init cmds
 ├─ i2c_bsp_init()                           ← I2C bus + device handles
 ├─ lcd_touch_init()                         ← CST816 initialisation
 ├─ i2c_drv2605_setup()                      ← DRV2605 ERM motor setup
 ├─ user_encoder_init()                      ← Encoder GPIO + interrupt
 │
 ├─ lv_init()                                ← LVGL core init
 ├─ lv_disp_draw_buf_init(...)               ← LVGL draw buffer (1/10 screen)
 ├─ lv_disp_drv_register(...)                ← Register flush/touch callbacks
 │
 ├─ esp_timer_create(lvgl_tick)              ← 2 ms LVGL tick timer
 │
 ├─ ui_init()                                ← SquareLine UI init
 │   ├─ ui_Menu_screen_init()
 │   └─ ui_LedColor_screen_init()
 │
 └─ xTaskCreate × 3:
     ├─ encoder_task          (core 0, pri 3)
     ├─ example_lvgl_port_task (core 1, pri 2)
     └─ i2c_drv2605_loop_task  (core 0, pri 1)
```

### LVGL Render Pipeline

```
esp_timer (2 ms) → lv_tick_inc()

example_lvgl_port_task:
  loop:
    xSemaphoreTake(lvgl_mux)
    lv_timer_handler()          ← processes dirty areas
    xSemaphoreGive(lvgl_mux)
    vTaskDelay(task_delay_ms)

example_lvgl_flush_cb:
  (called by LVGL when area is ready)
  → esp_lcd_panel_draw_bitmap() ← DMA transfer to SH8601
  → example_notify_lvgl_flush_ready() ← signals LVGL flush done
```

### Encoder Input Flow

```
Encoder ISR → sets bits in knob_even_ EventGroup

encoder_task:
  xEventGroupWaitBits(knob_even_, 0x07)
  │
  ├─ bit 0x01 (CCW):
  │   ui_Menu  → lv_roller_set_selected(current - 1)
  │   ui_LedColor → ui_LedColor_update_color(+10)
  │
  ├─ bit 0x02 (CW):
  │   ui_Menu  → lv_roller_set_selected(current + 1)
  │   ui_LedColor → ui_LedColor_update_color(-10)
  │
  └─ bit 0x04 (press): (available, currently unused in main app)
```

---

## 5. Directory Structure

```
knobui/
├── CMakeLists.txt              # Top-level project CMake
├── partitions.csv              # Custom partition table (NVS+PHY+7MB app)
├── sdkconfig.defaults          # Required ESP-IDF/LVGL Kconfig values
├── sdkconfig.basic             # Minimal baseline config
├── idf_component.yml           # (in main/) Managed component manifest
│
├── main/                       # Application code
│   ├── main.c                  # Entry point, hardware init, task creation
│   ├── user_config.h           # ALL pin definitions + compile flags
│   ├── lcd_driver.c/.h         # Low-level LCD test driver (legacy)
│   ├── basic_test.c/.h         # Basic hardware test routines
│   ├── lv_conf.h               # LVGL configuration (local copy)
│   │
│   ├── ui/                     # SquareLine Studio generated UI core
│   │   ├── ui.c                # Animations (sec/min/hour/cloud/heart...)
│   │   ├── ui.h                # Master UI header, screen includes
│   │   ├── ui_events.c         # Navigation event callbacks (EDIT HERE)
│   │   ├── ui_events.h
│   │   ├── ui_helpers.c/.h     # LVGL animation helper utilities
│   │   ├── components/         # Reusable UI components
│   │   │   ├── ui_comp_batterygroup.*
│   │   │   ├── ui_comp_buttondown.*
│   │   │   ├── ui_comp_buttonround.*
│   │   │   ├── ui_comp_buttontop.*
│   │   │   ├── ui_comp_dailymissiongroup.*
│   │   │   ├── ui_comp_dategroup.*
│   │   │   ├── ui_comp_forecastgroup.*
│   │   │   ├── ui_comp_hook.*
│   │   │   ├── ui_comp_pulsegroup.*
│   │   │   ├── ui_comp_stepgroup.*
│   │   │   ├── ui_comp_titlegroup.*
│   │   │   ├── ui_comp_todayweathergroup.*
│   │   │   └── ui_comp_weathergroup1.*
│   │   ├── fonts/              # Custom LVGL fonts (C arrays)
│   │   │   ├── ui_font_H1.c
│   │   │   ├── ui_font_Number_big.c
│   │   │   ├── ui_font_Number_extra.c
│   │   │   ├── ui_font_Subtitle.c
│   │   │   └── ui_font_Title.c
│   │   ├── images/             # PNG assets converted to C arrays
│   │   └── screens/            # Individual screen implementations
│   │       ├── ui_Menu.c/.h    # Main menu (infinite roller)
│   │       └── ui_LedColor.c/.h # Color wheel screen
│   │
│   └── CMakeLists.txt          # Glob all ui/*.c, link BSP components
│
├── components/                  # Custom ESP-IDF components
│   ├── i2c_bsp/                # I2C bus abstraction
│   ├── i2c_equipment/          # DRV2605 haptic driver (C++)
│   ├── lcd_bl_pwm_bsp/         # Backlight PWM (LEDC)
│   ├── lcd_touch_bsp/          # CST816 touch driver
│   ├── user_encoder_bsp/       # Rotary encoder (GPIO + EventGroup)
│   ├── SensorLib/              # Third-party sensor library (DRV2605 C++ class)
│   └── lv_conf.h               # LVGL config (component-level)
│
├── managed_components/          # ESP Component Manager dependencies
│   ├── espressif__cmake_utilities/
│   ├── espressif__esp_lcd_sh8601/   # SH8601 LCD panel driver
│   └── lvgl__lvgl/                  # LVGL 8.3.11
│
├── demo/                        # Standalone demo programs
│   ├── ESP-IDF/
│   │   ├── 01_ADC_Test/
│   │   ├── 02_SD_Card/
│   │   ├── 03_DRV2605_Test/
│   │   ├── 04_Encoder_Test/
│   │   ├── 05_WIFI_AP/
│   │   ├── 06_WIFI_STA/
│   │   ├── 07_Audio_Test/
│   │   └── 08_LVGL_Test/
│   └── Arduino/
│
└── ui_files/                    # SquareLine Studio project files
    ├── Smartwatch.spj           # SLS project file
    ├── Smartwatch.sll           # SLS layout
    ├── Smartwatch_events.py     # SLS event definitions
    ├── Themes.slt               # SLS theme definitions
    └── assets/                  # Source images/fonts for SLS
```

---

## 6. Components Reference

### `i2c_bsp` — I2C Bus

Initialises the ESP32-S3 I2C master and registers device handles.

```c
void i2c_bsp_init(void);
// Also exposes: drv2605_dev_handle (for i2c_equipment)
```

**Usage**: Called once from `app_main` before any I2C device drivers.

---

### `lcd_touch_bsp` — CST816 Touch Controller

```c
void lcd_touch_init(void);
uint8_t tpGetCoordinates(uint16_t *x, uint16_t *y);
// Returns: 1 if touch active, 0 if released
```

**LVGL integration**: `example_lvgl_touch_cb` polls `tpGetCoordinates` and feeds `lv_indev_data_t`. Rotation-aware when `EXAMPLE_Rotate_90` is defined.

---

### `lcd_bl_pwm_bsp` — Backlight PWM

```c
void lcd_bl_pwm_bsp_init(lcd_pwm_mode_t mode);
void setUpduty(lcd_pwm_mode_t mode);

// Modes: LCD_PWM_MODE_0, LCD_PWM_MODE_50, LCD_PWM_MODE_100,
//        LCD_PWM_MODE_150, LCD_PWM_MODE_200, LCD_PWM_MODE_255
```

Uses the ESP32-S3 LEDC peripheral on **GPIO 47**.

---

### `user_encoder_bsp` — Rotary Encoder

```c
void user_encoder_init(void);
extern EventGroupHandle_t knob_even_;
```

Configures GPIO 7 (ECB) and GPIO 8 (ECA) with edge-triggered interrupts. Sets bits in `knob_even_`:

| Bit | Direction |
|-----|-----------|
| `BIT(0)` = `0x01` | Counter-clockwise (CCW) |
| `BIT(1)` = `0x02` | Clockwise (CW) |
| `BIT(2)` = `0x04` | Button press |

**Consumer**: `encoder_task` in `main.c` calls `xEventGroupWaitBits(knob_even_, 0x07, pdTRUE, ...)`.

---

### `i2c_equipment` — DRV2605 Haptic Driver

This component wraps the `SensorDRV2605` C++ class with a C-compatible API.

```c
void i2c_drv2605_setup(void);
// Init: library 5, ERM mode, I2C trigger

void i2c_drv2605_haptic_feedback(uint8_t effect);
// Play waveform library effect (1–123)
// Effect 10 = "Double Click 100%"

void i2c_drv2605_haptic_feedback_strong(uint8_t intensity);
// Real-time mode: intensity 0–255, 100 ms duration

void i2c_drv2605_loop_task(void *arg);
// FreeRTOS task — runs haptic sequences
```

**Note**: The `.cpp` file requires `extern "C"` guards. The `i2c_dev_Callback` bridges the C++ SensorLib to the ESP-IDF I2C master API.

---

### `SensorLib` — Third-Party Sensor Library

Contains `SensorDRV2605.hpp`, a C++ driver class for the DRV2605 haptic controller. Used exclusively by `i2c_equipment.cpp`.

---

## 7. UI System

### SquareLine Studio Integration

The UI is designed in **SquareLine Studio 1.5.3** and exported to `main/ui/`. The project file is at `ui_files/Smartwatch.spj`.

**Re-export workflow**:
1. Edit design in SquareLine Studio.
2. Export → ESP-IDF → output to `main/ui/` and `main/ui/screens/`.
3. Any custom logic in `ui_events.c` must be re-applied after export (mark with `// CUSTOM:`).

### Screens

#### `ui_Menu` (Main Screen)

- **Widget**: `lv_roller` (infinite mode)
- **Options**: Clock | Lighting | Empty 1 | Empty 2 | Empty 3 | Empty 4
- **Encoder**: CCW/CW scrolls the roller up/down
- **Touch**: Tap a roller item triggers `OpenLedScreen()` if Lighting is selected (index 1)
- **Style**: Black background, Montserrat 48, grey text

#### `ui_LedColor` (Color Picker)

- **Widget**: `lv_colorwheel` (350×350, centered)
- **Encoder**: Adjusts hue via `ui_LedColor_update_color(±10)`
- **Back button**: Top-left corner, returns to `ui_Menu` via `ReturnToMenu()`
- **Style**: Black background

### LVGL Animations

Defined in `ui/ui.c`, these animations are declared and used by watch-face screens:

| Function | Duration | Range | Purpose |
|----------|----------|-------|---------|
| `sec_Animation` | 36,000 ms | 0–3600 | Second hand rotation (linear, infinite) |
| `min_Animation` | 1,000 ms | 0–1400 | Minute hand tick (ease-out) |
| `hour_Animation` | 1,000 ms | 0–300 | Hour hand tick (ease-out) |
| `left_Animation` | — | — | Slide-in from left |
| `right_Animation` | — | — | Slide-in from right |
| `opa_on_Animation` | — | — | Fade in |
| `dots_Animation` | — | — | Dot pulsing |
| `top_Animation` | — | — | Slide from top |
| `cloud_Animation` | — | — | Cloud float |
| `blood1/2_Animation` | — | — | Pulse/heartrate |
| `heart_Animation` | — | — | Heart beat |

### Custom Fonts

| Font | File | Usage |
|------|------|-------|
| `ui_font_H1` | `fonts/ui_font_H1.c` | Large heading |
| `ui_font_Number_big` | `fonts/ui_font_Number_big.c` | Large numbers |
| `ui_font_Number_extra` | `fonts/ui_font_Number_extra.c` | Extra-large numbers |
| `ui_font_Subtitle` | `fonts/ui_font_Subtitle.c` | Subtitle text |
| `ui_font_Title` | `fonts/ui_font_Title.c` | Title text |

### UI Components (Reusable)

| Component | Purpose |
|-----------|---------|
| `ui_comp_batterygroup` | Battery level display group |
| `ui_comp_buttondown/round/top` | Styled button variants |
| `ui_comp_dailymissiongroup` | Daily goals/missions |
| `ui_comp_dategroup` | Date display |
| `ui_comp_forecastgroup` | Weather forecast |
| `ui_comp_hook` | Generic hook container |
| `ui_comp_pulsegroup` | Heart rate / pulse |
| `ui_comp_stepgroup` | Step counter |
| `ui_comp_titlegroup` | Title bar |
| `ui_comp_todayweathergroup` | Today's weather |
| `ui_comp_weathergroup1` | Extended weather |

---

## 8. FreeRTOS Task Model

| Task | Stack | Priority | Core | Function |
|------|-------|----------|------|----------|
| `lvgl_task` | 4 KB | 2 | Any | `example_lvgl_port_task` — LVGL timer handler |
| `encoder_task` | default | 3 | Any | Polls encoder EventGroup, updates UI |
| `drv2605_loop` | default | 1 | Any | Haptic effect sequencer |

### LVGL Thread Safety

LVGL is **not thread-safe**. All LVGL API calls outside `example_lvgl_port_task` **must** use the mutex:

```c
if (example_lvgl_lock(-1)) {
    // safe to call any lv_* function here
    lv_roller_set_selected(ui_MenuRoller, new_index, LV_ANIM_ON);
    example_lvgl_unlock();
}
```

The `encoder_task` currently calls LVGL APIs directly — this works because the encoder task and LVGL task are never truly concurrent on the same LVGL state due to FreeRTOS scheduling, but for robustness the mutex should be acquired.

### LVGL Tick

An `esp_timer` fires every **2 ms** calling `lv_tick_inc(2)`. This drives LVGL's internal timing for animations and debounce.

---

## 9. Build Configuration

### `sdkconfig.defaults`

Key settings required for this project:

```kconfig
CONFIG_IDF_TARGET="esp32s3"
CONFIG_ESPTOOLPY_FLASHMODE_QIO=y        # QIO flash mode
CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y        # 8 MB flash
CONFIG_PARTITION_TABLE_CUSTOM=y         # Use partitions.csv
CONFIG_COMPILER_OPTIMIZATION_PERF=y     # -O2 optimisation

# PSRAM
CONFIG_SPIRAM=y
CONFIG_SPIRAM_MODE_OCT=y                # Octal PSRAM
CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y
CONFIG_SPIRAM_RODATA=y
CONFIG_SPIRAM_SPEED_80M=y
CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=10000

# CPU
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y   # 240 MHz
CONFIG_ESP32S3_DATA_CACHE_LINE_64B=y

# FreeRTOS
CONFIG_FREERTOS_HZ=1000                 # 1 ms tick

# LVGL
CONFIG_LV_COLOR_16_SWAP=y
CONFIG_LV_COLOR_SCREEN_TRANSP=y
CONFIG_LV_MEM_CUSTOM=y
CONFIG_LV_ATTRIBUTE_FAST_MEM_USE_IRAM=y
CONFIG_LV_DISP_DEF_REFR_PERIOD=20
CONFIG_LV_INDEV_DEF_READ_PERIOD=20
CONFIG_LV_FONT_MONTSERRAT_48=y          # Used by menu roller
# ... plus Montserrat 12,16,20,24,28,32,36,40,44
```

### Compile-Time Flags (`user_config.h`)

| Macro | Default | Effect |
|-------|---------|--------|
| `LOCAL_UI` | `1` | Include SquareLine UI; comment out for test mode |
| `EXAMPLE_USE_TOUCH` | `1` | Enable capacitive touch input |
| `EXAMPLE_Rotate_90` | commented | Rotate display + touch coords 90° |
| `Backlight_Testing` | commented | Start backlight PWM test task |

### `lv_conf.h`

Located at both `main/lv_conf.h` and `components/lv_conf.h`. Key values must match `sdkconfig.defaults`:

```c
#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0   // SquareLine requires this to be 0
```

---

## 10. Build & Flash Workflow

### Prerequisites

- ESP-IDF v5.2+ installed and sourced
- VS Code with ESP-IDF Extension (optional)
- USB-C cable connected to board

### Command Line

```bash
# One-time target setup
idf.py set-target esp32s3

# Build
idf.py build

# Flash
idf.py flash

# Flash + monitor serial output
idf.py flash monitor

# Flash at lower baud (for problematic connections)
idf.py -b 115200 flash

# Clean build
idf.py clean
```

### VS Code Tasks (`.vscode/tasks.json`)

| Task | Action |
|------|--------|
| `ESP-IDF: Build` | `idf.py build` |
| `ESP-IDF: Flash` | Build then `idf.py flash` |
| `ESP-IDF: Flash Low Baud` | Build then `idf.py -b 115200 flash` |
| `ESP-IDF: Monitor` | `idf.py monitor` (background) |
| `ESP-IDF: Flash and Monitor` | Build, flash, then monitor |
| `ESP-IDF: Clean` | `idf.py clean` |
| `ESP-IDF: Set Target ESP32-S3` | `idf.py set-target esp32s3` |
| `ESP-IDF: Menuconfig` | `idf.py menuconfig` |

### Expected Boot Log

```
I (counter): Initialize SPI bus
I (counter): Install panel IO
I (counter): Starting DRV2605 initialization...
I (i2c_equipment): DRV2605 begin successful
I (i2c_equipment): DRV2605 library 5 selected
I (i2c_equipment): DRV2605 initialization complete with ERM motor mode
I (counter): Starting LVGL task
I (counter): Starting encoder task
```

---

## 11. Demo Examples

The `demo/ESP-IDF/` directory contains independent standalone projects demonstrating individual peripherals.

| Demo | Description |
|------|-------------|
| `01_ADC_Test` | Read ADC channels (battery voltage) |
| `02_SD_Card` | MicroSD card read/write via SDMMC |
| `03_DRV2605_Test` | Play all 123 haptic waveform effects |
| `04_Encoder_Test` | Print encoder direction to serial |
| `05_WIFI_AP` | Start a Wi-Fi access point |
| `06_WIFI_STA` | Connect to Wi-Fi as station |
| `07_Audio_Test` | Play audio via PCM5100A over I2S |
| `08_LVGL_Test` | Standalone LVGL display demo |

Each demo is a self-contained ESP-IDF project with its own `CMakeLists.txt` and `main/` directory. Build them the same way as the main project.

---

## 12. Adding New Features

### Adding a New Screen

1. Create `main/screens/ui_NewScreen.c` and `ui_NewScreen.h`:

```c
// ui_NewScreen.h
#ifndef UI_NEWSCREEN_H
#define UI_NEWSCREEN_H
#include "../ui.h"
extern lv_obj_t *ui_NewScreen;
void ui_NewScreen_screen_init(void);
void ui_NewScreen_screen_destroy(void);
#endif
```

2. Include in `main/ui/ui.h`:

```c
#include "screens/ui_NewScreen.h"
```

3. Add init/destroy in `ui/ui.c`:

```c
void ui_init(void) {
    ui_Menu_screen_init();
    ui_LedColor_screen_init();
    ui_NewScreen_screen_init(); // CUSTOM: new screen
    lv_disp_load_scr(ui_Menu);
}
```

4. Add navigation in `ui_events.c`:

```c
// CUSTOM: navigate to new screen from menu index 2
case 2:
    lv_disp_load_scr(ui_NewScreen);
    break;
```

5. Add encoder handling in `main.c` → `encoder_task()`:

```c
if (lv_scr_act() == ui_NewScreen) {
    if (events & 0x01) { /* CCW action */ }
    if (events & 0x02) { /* CW action */ }
}
```

---

### Adding a New I2C Peripheral

1. Add I2C device registration in `components/i2c_bsp/i2c_bsp.c`:

```c
// Declare handle globally
i2c_master_dev_handle_t my_device_handle = NULL;

// In i2c_bsp_init():
i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = 0xXX,  // device I2C address
    .scl_speed_hz = 400000,
};
ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &my_device_handle));
```

2. Add setup/API in `components/i2c_equipment/`:

```c
// i2c_equipment.h — add declarations
void my_device_setup(void);
void my_device_action(void);

// i2c_equipment.cpp — implement using i2c_read_buff / i2c_write_buff
```

3. Call `my_device_setup()` from `app_main()` after `i2c_bsp_init()`.

---

### Triggering Haptic Feedback

```c
#include "i2c_equipment.h"

// Quick effect (non-blocking, from non-ISR context)
i2c_drv2605_haptic_feedback(1);   // Soft bump
i2c_drv2605_haptic_feedback(10);  // Double click (strong)
i2c_drv2605_haptic_feedback(14);  // Strong buzz

// Direct intensity (blocks for 100 ms)
i2c_drv2605_haptic_feedback_strong(200);  // 200/255 intensity
```

Common DRV2605 library 5 effects:

| Effect # | Description |
|----------|-------------|
| 1 | Strong Click 100% |
| 10 | Double Click 100% |
| 14 | Strong Buzz 100% |
| 58 | Transition Ramp Up Long Smooth 1 |
| 84 | Short Double Click Strong 1 |

---

### Adjusting Backlight Brightness

```c
#include "lcd_bl_pwm_bsp.h"

setUpduty(LCD_PWM_MODE_255);  // 100% (default)
setUpduty(LCD_PWM_MODE_200);  // ~78%
setUpduty(LCD_PWM_MODE_150);  // ~59%
setUpduty(LCD_PWM_MODE_100);  // ~39%
setUpduty(LCD_PWM_MODE_50);   // ~20%
setUpduty(LCD_PWM_MODE_0);    // Off
```

---

### Using LVGL from a Custom Task

Always acquire the mutex before any LVGL API:

```c
#include "freertos/semphr.h"
// lvgl_mux is defined in main.c — expose via header if needed

extern SemaphoreHandle_t lvgl_mux;  // or use the lock/unlock helpers

// Safe LVGL call from any task:
if (xSemaphoreTake(lvgl_mux, pdMS_TO_TICKS(500)) == pdTRUE) {
    lv_label_set_text(my_label, "Updated");
    xSemaphoreGive(lvgl_mux);
}
```

---

## 13. Troubleshooting

### Display shows nothing / white screen

- Verify `CONFIG_ESPTOOLPY_FLASHMODE_QIO=y` in sdkconfig.
- Check SPI GPIO definitions match the board — particularly that DATA0–DATA3 = GPIOs 15, 16, 17, 18.
- Confirm `lcd_bl_pwm_bsp_init(LCD_PWM_MODE_255)` is called before `lv_init()`.
- Ensure `sdkconfig.defaults` is applied: run `idf.py set-target esp32s3` to regenerate sdkconfig.

### Touch not responding

- Verify I2C is initialised (`i2c_bsp_init()` called first).
- Check `EXAMPLE_TOUCH_ADDR = 0x15` matches actual CST816 address.
- Confirm `EXAMPLE_USE_TOUCH 1` is defined in `user_config.h`.
- Test with `04_Encoder_Test` demo to isolate encoder vs touch issues.

### DRV2605 init fails ("drv2605 init failure")

- Check I2C address `0x5A` — verify with an I2C scanner sketch.
- Ensure `i2c_bsp_init()` runs before `i2c_drv2605_setup()`.
- Check the motor is physically connected to the DRV2605 output pins.

### Encoder not detected / wrong direction

- Swap GPIO 7 and GPIO 8 assignments in `user_config.h` to reverse direction.
- Verify encoder common/GND is connected.
- Check `knob_even_` is initialized before `encoder_task` starts.

### Build fails: "LV_COLOR_DEPTH should be 16bit"

- This assertion is in `main/ui/ui.c`. Ensure `CONFIG_LV_COLOR_DEPTH_16=y` in sdkconfig (set via `CONFIG_LV_COLOR_16_SWAP=y` path or menuconfig).

### Build fails: undefined font symbol

- Add the required font to `sdkconfig.defaults`: e.g., `CONFIG_LV_FONT_MONTSERRAT_48=y`
- Run `idf.py set-target esp32s3` to regenerate `sdkconfig`.

### Flash fails / connection errors

- Use the `ESP-IDF: Flash Low Baud` task (`idf.py -b 115200 flash`).
- Hold BOOT button while connecting, then release after `idf.py flash` starts.
- On macOS, verify port permissions: `ls -la /dev/cu.usbserial*`

### Out of memory / heap allocation fails

- Allocate large buffers in PSRAM: `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`.
- Verify `CONFIG_SPIRAM=y` and `CONFIG_SPIRAM_MODE_OCT=y`.
- Reduce `EXAMPLE_LVGL_BUF_HEIGHT` (currently `LCD_V_RES / 10`) to save internal RAM.

---

## Appendix: Key API Quick Reference

```c
// Backlight
void lcd_bl_pwm_bsp_init(lcd_pwm_mode_t mode);
void setUpduty(lcd_pwm_mode_t mode);

// Touch
void lcd_touch_init(void);
uint8_t tpGetCoordinates(uint16_t *x, uint16_t *y);

// I2C
void i2c_bsp_init(void);

// Haptic
void i2c_drv2605_setup(void);
void i2c_drv2605_haptic_feedback(uint8_t effect);
void i2c_drv2605_haptic_feedback_strong(uint8_t intensity);

// Encoder
void user_encoder_init(void);
extern EventGroupHandle_t knob_even_;  // bits: 0x01 CCW, 0x02 CW, 0x04 press

// UI
void ui_init(void);
void ui_destroy(void);
// Screens
extern lv_obj_t *ui_Menu;
extern lv_obj_t *ui_LedColor;
// Roller widget
extern lv_obj_t *ui_MenuRoller;
```
