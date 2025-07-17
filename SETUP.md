# ESP-IDF Installation and Setup Guide

## Prerequisites

This project requires ESP-IDF (Espressif IoT Development Framework) to be installed and configured properly.

## Installation Steps

### 1. Install ESP-IDF using VS Code Extension (Recommended)

1. **Open VS Code Command Palette** (`Ctrl+Shift+P`)
2. **Search for**: `ESP-IDF: Configure ESP-IDF extension`
3. **Select**: `EXPRESS` installation mode
4. **Choose**: ESP-IDF version 5.1 or later
5. **Select**: Installation directory (e.g., `C:\esp\esp-idf`)
6. **Wait**: For the installation to complete (this may take 15-30 minutes)

### 2. Alternative: Manual Installation

If the VS Code extension installation fails, install manually:

1. **Download**: [ESP-IDF Windows Installer](https://dl.espressif.com/dl/esp-idf/)
2. **Run**: The installer and follow the setup wizard
3. **Add**: ESP-IDF to system PATH
4. **Restart**: VS Code after installation

### 3. Verify Installation

Open a new PowerShell terminal and run:
```powershell
idf.py --version
```

You should see something like:
```
ESP-IDF v5.1.x
```

## Current Project Status

✅ **FLASHING SUCCESSFUL** - Timer alarm firmware has been successfully flashed to the ESP32 device.

⚠️ **Hardware Mismatch Detected**: 
- Your device is an **ESP32-U4WDH** (not ESP32-S3)
- Current configuration is set for ESP32-S3, causing PSRAM initialization errors
- Device reboots due to missing PSRAM hardware

### Immediate Next Steps

1. **Fix Target Configuration**:
   ```powershell
   # Remove current build
   Remove-Item -Path "build" -Recurse -Force -ErrorAction SilentlyContinue
   
   # Set correct target
   C:\Users\Zach\esp\v5.2.5\esp-idf\export.ps1
   idf.py set-target esp32
   ```

2. **Rebuild and Flash**:
   ```powershell
   idf.py build
   idf.py flash monitor
   ```

### Timer Alarm Features (Ready to Test)

Your timer alarm functionality is fully implemented and ready:

- **Visual Alarm**: Timer text turns red when timer ends
- **"Round Ended" Message**: Appears below the timer display  
- **2-Second Haptic Alarm**: Vibration pattern (200ms on, 100ms off, 200ms on, 300ms pause)
- **Automatic Reset**: Alarm clears when timer is restarted

## Project Setup

Once ESP-IDF is installed:

### 1. Set Target Device
```powershell
idf.py set-target esp32
```

**Note**: This project detects ESP32-U4WDH hardware, so we use `esp32` target (not `esp32s3`).

### 2. Build the Project
```powershell
idf.py build
```

### Flash to Device
Connect your ESP32 board via USB-C, then:
```powershell
idf.py flash
```

### 4. Monitor Serial Output
```powershell
idf.py monitor
```

Or combine flash and monitor:
```powershell
idf.py flash monitor
```

## VS Code Integration

### Using Command Palette
- `Ctrl+Shift+P` → `ESP-IDF: Build your project`
- `Ctrl+Shift+P` → `ESP-IDF: Flash your project`
- `Ctrl+Shift+P` → `ESP-IDF: Monitor your device`

### Using Tasks
- `Ctrl+Shift+P` → `Tasks: Run Task` → Select desired ESP-IDF task

## Hardware Setup

### ESP32-Knob-Touch-LCD-1.8 Connections

**Hardware Detected**: ESP32-U4WDH (revision v3.0) with WiFi and BT capabilities

**Important**: The GPIO pin assignments in the code may need adjustment based on your specific board's pinout. Verify these connections:

- **LCD SPI**:
  - MOSI: GPIO 11
  - CLK: GPIO 12
  - CS: GPIO 10
  - DC: GPIO 13
  - RST: GPIO 14
  - BL: GPIO 15

### USB Connection
- Use a good quality USB-C cable
- Try different orientations if the device is not detected
- Some boards require holding BOOT button while connecting

## Troubleshooting

### ESP-IDF Not Found
- Ensure ESP-IDF is properly installed
- Check that `idf.py` is in your system PATH
- Restart VS Code after installation
- Try opening ESP-IDF Terminal from VS Code

### Build Errors
- Verify target is set to `esp32` (not `esp32s3`)
- Check that all required components are available
- Try `idf.py clean` then `idf.py build`

### Flash Errors
- Check USB cable and connection
- Try different USB-C orientations
- Ensure device drivers are installed
- Hold BOOT button if necessary during flashing

### No Display Output
- Verify GPIO pin assignments match your board
- Check power connections
- Ensure LCD is properly connected
- Review serial monitor for error messages

## Next Steps

Once the basic test is working:

1. **Touch Input**: Add touch controller (CST816) support
2. **Encoder**: Implement rotary encoder handling
3. **Audio**: Add I2S audio codec support
4. **LVGL**: Integrate LVGL for advanced GUI
5. **Wi-Fi**: Add wireless connectivity
6. **SD Card**: Implement file system support

## Resources

- [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)
- [Waveshare ESP32-S3 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Knob-Touch-LCD-1.8)
- [ESP32-S3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
