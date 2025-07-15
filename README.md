# ESP32-S3 Knob Touch LCD Project

A bare-bones ESP-IDF project for the **Waveshare ESP32-S3-Knob-Touch-LCD-1.8** development board that displays a simple test message on the LCD screen.

## 🚀 Quick Start

### Prerequisites
- **VS Code** with **ESP-IDF Extension**
- **ESP32-S3-Knob-Touch-LCD-1.8** board
- **USB-C cable**

### 1. Install ESP-IDF (First Time Only)
```
1. Press Ctrl+Shift+P in VS Code
2. Type: "ESP-IDF: Configure ESP-IDF extension"
3. Choose "EXPRESS" installation
4. Wait ~15-30 minutes for download
5. Restart VS Code when complete
```

### 2. Verify Installation
```cmd
.\verify-esp-idf.bat
```
Should show: `ESP-IDF Installation Status: READY ✅`

### 3. Build & Flash
```cmd
.\build.bat
```
Or manually:
```bash
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

### 4. Expected Result
The 1.8" LCD (360x360) should display:
- **"HELLO WORLD!"** (white text)
- **"ESP LCD TEST"** (cyan text)  
- **"STATUS OK"** (green text, blinking every 2 seconds)
- **Three colored rectangles** (red, green, blue)
- **White border** around the screen

## 📁 Project Structure

```
├── main/                   # Main application code
│   ├── main.c             # Application entry point + LCD test
│   ├── lcd_driver.c       # LCD driver implementation  
│   ├── lcd_driver.h       # LCD driver header
│   └── CMakeLists.txt     # Component build configuration
├── .vscode/               # VS Code settings
│   ├── tasks.json         # Build/flash/monitor tasks
│   ├── settings.json      # C/C++ IntelliSense settings
│   └── launch.json        # Debug configuration
├── .github/
│   └── copilot-instructions.md  # AI coding assistant guidance
├── CMakeLists.txt         # Project build configuration
├── sdkconfig.defaults     # ESP32-S3 optimized settings
├── build.bat             # Quick build script
├── verify-esp-idf.bat    # Installation verification
├── validate.py           # Project structure validator
├── SETUP.md              # Detailed setup instructions
└── README.md             # This file
```

## 🛠️ Development

### VS Code Integration
- **Build**: `Ctrl+Shift+P` → `ESP-IDF: Build your project`
- **Flash**: `Ctrl+Shift+P` → `ESP-IDF: Flash your project`
- **Monitor**: `Ctrl+Shift+P` → `ESP-IDF: Monitor your device`
- **Set Target**: `Ctrl+Shift+P` → `ESP-IDF: Set Espressif device target`

### Available Scripts
- `.\build.bat` - Quick build and setup
- `.\verify-esp-idf.bat` - Check ESP-IDF installation
- `python validate.py` - Validate project structure

## 🔧 Hardware Configuration

### LCD SPI Interface (Adjust for your board)
```c
#define LCD_PIN_NUM_MOSI    11  // SPI Data
#define LCD_PIN_NUM_CLK     12  // SPI Clock  
#define LCD_PIN_NUM_CS      10  // Chip Select
#define LCD_PIN_NUM_DC      13  // Data/Command
#define LCD_PIN_NUM_RST     14  // Reset
#define LCD_PIN_NUM_BL      15  // Backlight (PWM)
```

**⚠️ Important**: These GPIO assignments may need adjustment. Check the [Waveshare wiki](https://www.waveshare.com/wiki/ESP32-S3-Knob-Touch-LCD-1.8) for your board's exact pinout.

### Device Specifications
- **MCU**: ESP32-S3 (dual-core, 240MHz)
- **Display**: 1.8" LCD, 360x360 resolution, ST7789 controller
- **Touch**: Capacitive touch (CST816) - *not implemented yet*
- **Audio**: PCM5100A codec - *not implemented yet*
- **Input**: Rotary encoder - *not implemented yet*
- **Storage**: MicroSD card slot - *not implemented yet*

## 🐛 Troubleshooting

### ESP-IDF Issues
```bash
# Check installation
.\verify-esp-idf.bat

# If not found, install via VS Code:
# Ctrl+Shift+P → "ESP-IDF: Configure ESP-IDF extension"
```

### Build Errors
```bash
# Clean and rebuild
idf.py clean
idf.py build
```

### Flash Issues
- Try different USB-C cable orientations
- Hold **BOOT** button while connecting
- Check Device Manager for COM port
- Ensure adequate power supply

### No Display Output
- Verify GPIO pin assignments match your board
- Check LCD connections and power
- Review serial monitor for initialization errors
- Ensure ST7789 controller compatibility

## 🚀 Next Steps

This bare-bones framework can be extended with:

- **🎛️ Touch Input**: CST816 capacitive touch integration
- **🔄 Rotary Encoder**: Button and rotation handling
- **🔊 Audio**: I2S audio playback/recording (PCM5100A)
- **📶 Wi-Fi**: Wireless connectivity
- **🎨 LVGL**: Advanced GUI framework  
- **💾 File System**: SD card file operations
- **📳 Haptic**: Vibration motor feedback (DRV2605)

## 📚 Resources

- **[Detailed Setup Guide](SETUP.md)** - Step-by-step installation
- **[Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-S3-Knob-Touch-LCD-1.8)** - Hardware documentation
- **[ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)** - Framework reference
- **[ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)** - Chip specifications

## 📄 License

This project is open source and available under the MIT License.
