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

## Project Setup

Once ESP-IDF is installed:

### 1. Set Target Device
```powershell
idf.py set-target esp32s3
```

### 2. Build the Project
```powershell
idf.py build
```

### 3. Flash to Device
Connect your ESP32-S3 board via USB-C, then:

**🔧 NEW: Reliable Flash Method (Recommended)**
```powershell
.\flash-fix.ps1
```

**Or use VS Code Task:**
- `Ctrl+Shift+P` → `Tasks: Run Task` → `ESP-IDF: Flash (Reliable)`

**Or manual command:**
```powershell
idf.py -b 115200 flash
```

**💡 Tip**: If flashing fails, hold BOOT button while connecting USB-C

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

### ESP32-S3-Knob-Touch-LCD-1.8 Connections

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
- Verify target is set to `esp32s3`
- Check that all required components are available
- Try `idf.py clean` then `idf.py build`

### Flash Errors
**🔧 NEW: Most flash errors are now fixed with our reliable flash script!**

Try these solutions in order:
1. **Use reliable flash script**: `.\flash-fix.ps1`
2. **Hold BOOT button** while connecting USB-C
3. **Try lower baud rate**: `idf.py -b 115200 flash`
4. **Check USB cable** - some cables are power-only
5. **Try different USB port** - USB 3.0 sometimes has issues
6. **Restart VS Code** if COM port seems locked

**Common issues fixed:**
- Wrong flash size (was 16MB, now 8MB)
- High baud rate timeouts (now 115200)
- Partition table too big (now 7MB)
- No bootloader mode detection

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

## GitHub Integration

### Quick Updates
Use VS Code tasks for easy GitHub updates:
- `Ctrl+Shift+P` → `Tasks: Run Task` → `Git: Quick Update`
- `Ctrl+Shift+P` → `Tasks: Run Task` → `Git: Status`

### Manual Updates
```powershell
# Quick update with PowerShell script
.\quick-update.ps1 "Your commit message"

# Or manual git commands
git add .
git commit -m "Your changes description"
git push
```

### Repository URL
https://github.com/zwood425/swubasedatapad

## Resources

- [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)
- [Waveshare ESP32-S3 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Knob-Touch-LCD-1.8)
- [ESP32-S3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
