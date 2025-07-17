# ESP32-S3 Flash Fix Script
# This script addresses common daily flashing issues

Write-Host "🔧 ESP32-S3 Flash Fix Script" -ForegroundColor Green
Write-Host "Addressing common daily flashing issues..." -ForegroundColor Yellow

# 1. Check for device in bootloader mode
Write-Host "`n1. Checking for ESP32-S3 device..." -ForegroundColor Cyan
$comPorts = Get-CimInstance -ClassName Win32_SerialPort | Where-Object { $_.Description -like "*USB*" }
if ($comPorts) {
    Write-Host "Found USB serial devices:" -ForegroundColor Green
    $comPorts | ForEach-Object { 
        Write-Host "  - $($_.DeviceID): $($_.Description)" -ForegroundColor White
    }
} else {
    Write-Host "⚠️  No USB serial devices found!" -ForegroundColor Red
    Write-Host "Please:" -ForegroundColor Yellow
    Write-Host "  1. Connect ESP32-S3 via USB-C" -ForegroundColor Yellow
    Write-Host "  2. Hold BOOT button while connecting" -ForegroundColor Yellow
    Write-Host "  3. Try different USB cable/port" -ForegroundColor Yellow
    exit 1
}

# 2. Set up ESP-IDF environment
Write-Host "`n2. Setting up ESP-IDF environment..." -ForegroundColor Cyan
$espIdfPath = "C:\Users\Zach\esp\v5.2.5\esp-idf\export.ps1"
if (Test-Path $espIdfPath) {
    Write-Host "Loading ESP-IDF environment..." -ForegroundColor Green
    . $espIdfPath
} else {
    Write-Host "❌ ESP-IDF not found at: $espIdfPath" -ForegroundColor Red
    exit 1
}

# 3. Try flashing with different strategies
Write-Host "`n3. Attempting flash with optimized settings..." -ForegroundColor Cyan

# Strategy 1: Lower baud rate for reliability
Write-Host "Strategy 1: Lower baud rate (115200)" -ForegroundColor Yellow
$result1 = & idf.py -b 115200 flash 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Flash successful with lower baud rate!" -ForegroundColor Green
    exit 0
}

# Strategy 2: Specific COM port
$primaryPort = ($comPorts | Where-Object { $_.Description -like "*USB Serial*" } | Select-Object -First 1).DeviceID
if ($primaryPort) {
    Write-Host "Strategy 2: Specific port ($primaryPort)" -ForegroundColor Yellow
    $result2 = & idf.py -p $primaryPort -b 115200 flash 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Flash successful with specific port!" -ForegroundColor Green
        exit 0
    }
}

# Strategy 3: Manual esptool
Write-Host "Strategy 3: Direct esptool command" -ForegroundColor Yellow
if ($primaryPort) {
    $espToolCmd = "python -m esptool --chip esp32s3 -p $primaryPort -b 115200 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 8MB --flash_freq 80m 0x0 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0x10000 build\LVGL_Test.bin"
    Write-Host "Command: $espToolCmd" -ForegroundColor Gray
    $result3 = Invoke-Expression $espToolCmd 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Flash successful with direct esptool!" -ForegroundColor Green
        exit 0
    }
}

# If all strategies fail
Write-Host "`n❌ All flash strategies failed!" -ForegroundColor Red
Write-Host "Manual troubleshooting steps:" -ForegroundColor Yellow
Write-Host "1. Disconnect and reconnect device" -ForegroundColor White
Write-Host "2. Hold BOOT button while connecting" -ForegroundColor White
Write-Host "3. Try different USB cable/port" -ForegroundColor White
Write-Host "4. Check Device Manager for driver issues" -ForegroundColor White
Write-Host "5. Restart VS Code and try again" -ForegroundColor White
