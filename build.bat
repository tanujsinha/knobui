@echo off
echo ESP32-S3 Project Quick Build
echo ============================
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
echo Next steps:
echo 1. Connect ESP32-S3 board via USB-C
echo 2. Run: idf.py flash monitor
echo 3. Should see "HELLO WORLD!" on LCD display
echo.
pause
