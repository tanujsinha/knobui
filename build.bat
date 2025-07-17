@echo off
echo ESP32-S3 Project Build and Upload
echo ==================================
echo.

REM Check if ESP-IDF is available
where idf.py >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ ESP-IDF not installed yet!
    echo.
    echo Please install ESP-IDF first:
    echo 1. Press Ctrl+Shift+P in VS Code
    echo 2. Type: "ESP-IDF: Configure ESP-IDF extension"
    echo 3. Choose EXPRESS installation
    echo 4. Wait for installation to complete
    echo 5. Restart VS Code
    echo.
    echo Then run this script again.
    pause
    exit /b 1
)

echo ✅ ESP-IDF found!
echo.

echo Setting target to ESP32-S3...
idf.py set-target esp32s3
if %errorlevel% neq 0 (
    echo ❌ Failed to set target
    pause
    exit /b 1
)

echo.
echo Building project...
idf.py build
if %errorlevel% neq 0 (
    echo ❌ Build failed
    echo Check error messages above
    pause
    exit /b 1
)

echo.
echo ✅ Build successful!
echo.
echo Connect your ESP32-S3 device now:
echo 1. Connect via USB-C cable
echo 2. Try different orientations if not detected
echo 3. Hold BOOT button if necessary
echo.
set /p continue="Press Enter when device is connected..."

echo.
echo Flashing to device and starting monitor...
echo (Press Ctrl+] to exit monitor)
echo.
idf.py flash monitor

echo.
echo ================================
echo Upload process complete!
echo.
echo Expected results on LCD:
echo - "HELLO WORLD!" at top
echo - "ESP LCD TEST" in middle
echo - "STATUS OK" blinking every 2 seconds
echo - Colored rectangles and border
echo ================================
pause
