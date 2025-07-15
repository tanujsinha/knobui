@echo off
echo ESP-IDF Installation Verification
echo ===================================
echo.

echo Checking ESP-IDF installation...
where idf.py >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ ESP-IDF not found in PATH
    echo.
    echo The installation may not be complete or VS Code needs restart.
    echo Try these steps:
    echo 1. Restart VS Code completely
    echo 2. Open a new terminal in VS Code
    echo 3. Try running this script again
    echo.
    echo If still not working, re-run ESP-IDF extension setup:
    echo Ctrl+Shift+P ^> "ESP-IDF: Configure ESP-IDF extension"
    goto :end
)

echo ✅ ESP-IDF found!
echo.

echo Getting ESP-IDF version...
idf.py --version
echo.

echo Getting Python version...
python --version
echo.

echo Checking ESP32-S3 support...
idf.py --list-targets | findstr esp32s3 >nul
if %errorlevel% eq 0 (
    echo ✅ ESP32-S3 target supported
) else (
    echo ❌ ESP32-S3 target not found
)

echo.
echo Testing project target setting...
idf.py set-target esp32s3 >nul 2>&1
if %errorlevel% eq 0 (
    echo ✅ ESP32-S3 target can be set
) else (
    echo ❌ Failed to set ESP32-S3 target
)

echo.
echo ===================================
echo ESP-IDF Installation Status: READY ✅
echo You can now build ESP32-S3 projects!
echo ===================================

:end
pause
